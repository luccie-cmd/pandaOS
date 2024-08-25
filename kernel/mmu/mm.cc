    #include "mmu.h"
    #include <cstdio>
    #include <dbg/dbg.h>
    #include <io/io.h>
    #include <cstring>

    struct SAFreelist
    {
        std::size_t size;
        SAFreelist* next;
    };

    static SAFreelist* head = nullptr;

    void* MmSaAlloc(std::size_t size)
    {
        // Align the size to be a multiple of sizeof(SAFreelist*)
        size = (size + sizeof(SAFreelist) - 1) & ~(sizeof(SAFreelist) - 1);

        SAFreelist** current = &head;

        // Traverse the list to find a suitable block
        while (*current)
        {
            if ((*current)->size >= size)
            {
                SAFreelist* block = *current;

                // If the block is exactly the size needed, remove it from the list
                if (block->size == size)
                {
                    *current = block->next;
                }
                else
                {
                    // Split the block
                    SAFreelist* next_block = reinterpret_cast<SAFreelist*>(reinterpret_cast<uintptr_t>(block) + size);
                    next_block->size = block->size - size;
                    next_block->next = block->next;

                    *current = next_block;
                    block->size = size;
                }

                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block) + sizeof(SAFreelist));
            }

            current = &(*current)->next;
        }

        // No suitable block found, allocate a new page
        auto new_head_ptr = reinterpret_cast<uintptr_t>(mmu::allocatePage());
        new_head_ptr += mmu::getHHDM();

        SAFreelist* new_block = reinterpret_cast<SAFreelist*>(new_head_ptr);
        new_block->next = head;
        new_block->size = 4096;

        head = new_block;

        return MmSaAlloc(size); // Retry allocation
    }

    void MmSaFree(void* ptr, std::size_t size)
    {
        if (ptr == nullptr || size == 0)
            return;

        // Align the size to be a multiple of sizeof(SAFreelist*)
        size = (size + sizeof(SAFreelist) - 1) & ~(sizeof(SAFreelist) - 1);

        SAFreelist* block = reinterpret_cast<SAFreelist*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(SAFreelist));
        block->size = size;

        // Insert the block back into the freelist
        SAFreelist** current = &head;

        while (*current && (*current < block))
        {
            current = &(*current)->next;
        }

        block->next = *current;
        *current = block;

        // Merge adjacent free blocks
        if (block->next && reinterpret_cast<uintptr_t>(block) + block->size + sizeof(SAFreelist) == reinterpret_cast<uintptr_t>(block->next))
        {
            block->size += block->next->size + sizeof(SAFreelist);
            block->next = block->next->next;
        }

        if (current != &head && reinterpret_cast<uintptr_t>((*current)->next) == reinterpret_cast<uintptr_t>(block) + block->size + sizeof(SAFreelist))
        {
            (*current)->size += block->next->size + sizeof(SAFreelist);
            (*current)->next = block->next->next;
        }
    }

    namespace mmu{
        struct FreeListEntry{
            uint64_t length;
            FreeListEntry* next;
        };

        FreeListEntry* head = nullptr;

        limine_memmap_request memmap_request = {
            .id = LIMINE_MEMMAP_REQUEST,
            .revision = 0,
            .response = nullptr
        };
        limine_hhdm_request hhdm_request = {
            .id = LIMINE_HHDM_REQUEST,
            .revision = 0,
            .response = nullptr,
        };
        limine_kernel_address_request kernel_address_request = {
            .id = LIMINE_KERNEL_ADDRESS_REQUEST,
            .revision = 0,
            .response = nullptr
        };
        uint64_t hhdm_offset;
        uint64_t kernel_addr;
        struct VmFreelist
        {
            uint64_t base_address;
            uint64_t size;
            VmFreelist* next;
        };

        static VmFreelist* freelist_head = nullptr;
        limine_memmap_entry** entries;
        uint64_t entries_count;

        static void addEntryToFreelist(uint64_t base_address, uint64_t length)
        {
            VmFreelist* entry = (VmFreelist*) MmSaAlloc(sizeof(VmFreelist));
            entry->next = nullptr;
            entry->size = length;
            entry->base_address = base_address;

            if (freelist_head == nullptr)
            {
                freelist_head = entry;

                return;
            }

            entry->next = freelist_head;
            freelist_head = entry;
        }

        void init_mm(){
            if(memmap_request.response == nullptr){
                std::error("Memory request failed\n");
            }
            if(hhdm_request.response == nullptr){
                std::error("No HHDM was found\n");
            }
            if(kernel_address_request.response == nullptr){
                std::error("Kernel address could not be found\n");
            }

            kernel_addr = kernel_address_request.response->virtual_base;
            hhdm_offset = hhdm_request.response->offset;
            entries = memmap_request.response->entries;
            entries_count = memmap_request.response->entry_count;

            for (std::size_t entry_index = 0; entry_index < entries_count; ++entry_index){
                limine_memmap_entry* entry = entries[entry_index];
                if(entry->type == LIMINE_MEMMAP_USABLE){
                    uintptr_t base = static_cast<uintptr_t>(makeVirtual(entry->base));
                    std::size_t length = (std::size_t)entry->length;
                    // Avoid using extra memory on entries we can't even use
                    if(length > 0){
                        FreeListEntry* new_entry = reinterpret_cast<FreeListEntry*>(base);
                        new_entry->length = length;
                        new_entry->next = nullptr;

                        if(head == nullptr){
                            head = new_entry;
                        } else {
                            new_entry->next = head;
                            head = new_entry;
                        }
                    }
                }
            }
            auto memory_map = retrieveMemoryMap();
            auto memory_map_entry_count = retrieveMemoryMapEntryCount();

            // Find the highest address.
            uint64_t highest_address = 0;
            for (size_t entry_index = 0; memory_map_entry_count > entry_index; entry_index++)
            {
                auto this_entry = memory_map[entry_index];
                uint64_t top_address = this_entry->length + this_entry->base;

                if (top_address > highest_address)
                    highest_address = top_address;
            }

            uint64_t start_address = (hhdm_offset + highest_address + (0x1000 - 1)) & ~(0x1000 -1);
            uint64_t size = kernel_addr - start_address;

            addEntryToFreelist(start_address, size);
        }
        static const char* memmap_entry_type_to_cstr(uint64_t type){
            switch(type){
                case LIMINE_MEMMAP_USABLE: {
                    return "Useable";
                } break;
                case LIMINE_MEMMAP_RESERVED: {
                    return "Reserved";
                } break;
                case LIMINE_MEMMAP_ACPI_NVS: {
                    return "ACPI (Non volatile)";
                } break;
                case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: {
                    return "Bootloader";
                } break;
                case LIMINE_MEMMAP_KERNEL_AND_MODULES: {
                    return "Kernel";
                } break;
                case LIMINE_MEMMAP_FRAMEBUFFER: {
                    return "Framebuffer";
                } break;
            }
            return nullptr;
        }
        void printPmmInfo(){
            std::printf("  PMM:\n");
            for(uint64_t i = 0; i < entries_count; ++i){
                limine_memmap_entry* entry = entries[i];
                const char* type = memmap_entry_type_to_cstr(entry->type);
                if(type == nullptr){
                    std::error("Invalid limine memmap type `%ld`\n", entry->type);
                }
                std::printf("\t%s: %lx -> %lx\n", type, entry->base, entry->base+entry->length);
            }
        }
        uint64_t allocatePage(){
            FreeListEntry* previous = nullptr;
            FreeListEntry* current = head;

            while (current != nullptr){
                if (current->length >= 4096){
                    // Allocate the page
                    uintptr_t allocated_address = reinterpret_cast<uintptr_t>(current);

                    // If the block is larger than a page, reduce its size
                    if (current->length > 4096){
                        FreeListEntry* new_entry = reinterpret_cast<FreeListEntry*>(allocated_address + 4096);
                        new_entry->length = current->length - 4096;
                        new_entry->next = current->next;

                        if (previous != nullptr){
                            previous->next = new_entry;
                        }
                        else{
                            head = new_entry;
                        }
                    }
                    else{
                        // Remove the block from the free list
                        if (previous != nullptr){
                            previous->next = current->next;
                        }
                        else{
                            head = current->next;
                        }
                    }
                    return allocated_address - getHHDM();
                }

                previous = current;
                current = current->next;
            }

            std::error("No suitable block found.\n");
        }
        limine_memmap_entry** retrieveMemoryMap(){
            return entries;
        }
        uint64_t retrieveMemoryMapEntryCount(){
            return entries_count;
        }
        void freePage(uint64_t addr){
            uintptr_t base = reinterpret_cast<uintptr_t>(makeVirtual(addr));
            FreeListEntry* dealloc_entry = reinterpret_cast<FreeListEntry*>(base);
            dealloc_entry->length = 4096; // Assuming pages are always 4096 bytes

            FreeListEntry* current = head;
            FreeListEntry* previous = nullptr;

            while (current != nullptr && reinterpret_cast<uintptr_t>(current) < base){
                previous = current;
                current = current->next;
            }

            if (previous != nullptr){
                previous->next = dealloc_entry;
                dealloc_entry->next = current;
            }
            else{
                dealloc_entry->next = head;
                head = dealloc_entry;
            }
        }
        void printVmmInfo(){
            std::printf("  VMM:\n\tHHDM: %lx\n", hhdm_offset);
        }
        uint64_t makeVirtual(uint64_t addr){
            return addr+hhdm_offset;
        }
        pml4* getPML4()
        {
            uint64_t cr3_value = io::cr3();
            cr3_value &= ~(0xfff); // Mask out the flags to get PPN of PML4.
            cr3_value += hhdm_offset;
            return reinterpret_cast<pml4*>(cr3_value);
        }

        struct vmm_address
        {
            uint64_t padding : 16;
            uint64_t pml4e : 9;
            uint64_t pdpe : 9;
            uint64_t pde : 9;
            uint64_t pte : 9;
            uint64_t offset : 12;
        } __attribute__((packed));

        template <typename T>
        static constexpr vmm_address vmm_split_va(T va) {
            // Assuming vaddr is a virtual memory address represented as an integer type
            vmm_address result;

            uint64_t vaddr = reinterpret_cast<uint64_t>(va);

            // Extract different components of the virtual address
            result.padding = (vaddr >> 48) & 0xFFFF;
            result.pml4e = (vaddr >> 39) & 0x1FF;
            result.pdpe = (vaddr >> 30) & 0x1FF;
            result.pde = (vaddr >> 21) & 0x1FF;
            result.pte = (vaddr >> 12) & 0x1FF;
            result.offset = vaddr & 0xFFF;

            // Return the result
            return result;
        }
        uint64_t getHHDM(){
            return hhdm_offset;
        }
        bool mapMemory(pml4* pml4e, uint64_t virtAddr, uint64_t physAddr, int prot, int flags){
            if(pml4e == nullptr){
                return false;
            }
            vmm_address va = vmm_split_va(virtAddr);

            if(virtAddr % PAGE_SIZE != 0){
                std::error("KERNEL: Please allign virtPage size before calling internal kernel/mmu/vmm.cc %s attempted virtAddr = %ld\n", __PRETTY_FUNCTION__, virtAddr);
            }
            if(physAddr % PAGE_SIZE != 0){
                std::error("KERNEL: Please allign physPage size before calling internal kernel/mmu/vmm.cc %s attempted physAddr = %ld\n", __PRETTY_FUNCTION__, physAddr);
            }
            
            // Get separate flags already.
            bool protection_rw = (prot & PROTECTION_RW) > 0 ? true : false;
            bool protection_noexec = (prot & PROTECTION_NO_EXECUTE) > 0 ?  true : false;
            bool protection_supervisor = (prot & PROTECTION_KERNEL) > 0 ?  true : false;

            bool map_present = (flags & MAP_PRESENT) > 0 ?  true : false;
            bool map_global = (flags & MAP_GLOBAL) > 0 ?  true : false;

            if (!protection_rw) std::printf("WARNING! NO RW FLAG SET.\n");
            if (!map_present) std::printf("WARNING! NO PRESENT FLAG SET.\n");

            if (pml4e[va.pml4e].pdpe_ptr == 0) {
                uint64_t p = allocatePage();
                std::memset(reinterpret_cast<uint8_t*>(makeVirtual(p)), 0, 4096);
                pml4e[va.pml4e].pdpe_ptr = (uint64_t)p >> 12;
            }

            pdpe* pdpe = reinterpret_cast<struct pdpe*>(makeVirtual(pml4e[va.pml4e].pdpe_ptr << 12));
            if (pdpe[va.pdpe].pde_ptr == 0) {
                uint64_t p = allocatePage();
                std::memset(reinterpret_cast<uint8_t*>(makeVirtual(p)), 0, 4096);
                pdpe[va.pdpe].pde_ptr = (uint64_t)p >> 12;
            }

            pde* pde =  reinterpret_cast<struct pde*>(makeVirtual(pdpe[va.pdpe].pde_ptr << 12));
            if (pde[va.pde].pte_ptr == 0) {
                uint64_t p = allocatePage();
                std::memset(reinterpret_cast<uint8_t*>(makeVirtual(p)), 0, 4096);
                pde[va.pde].pte_ptr = (uint64_t)p >> 12;
            }

            pte* pte = reinterpret_cast<struct pte*>(makeVirtual(pde[va.pde].pte_ptr << 12));

            // Clear all fields and set only the specified flags
            pml4e[va.pml4e].present = map_present;
            pml4e[va.pml4e].rw = protection_rw;
            pml4e[va.pml4e].user = !protection_supervisor;
            pml4e[va.pml4e].no_execute = protection_noexec;
            // Clear unspecified flags
            pml4e[va.pml4e].pwt = 0;
            pml4e[va.pml4e].pcd = 0;
            pml4e[va.pml4e].accesed = 0;
            pml4e[va.pml4e].ignored = 0;
            pml4e[va.pml4e].mbz = 0;
            pml4e[va.pml4e].ats0 = 0;
            pml4e[va.pml4e].ats1 = 0;

            pdpe[va.pdpe].present = map_present;
            pdpe[va.pdpe].rw = protection_rw;
            pdpe[va.pdpe].user = !protection_supervisor;
            pdpe[va.pdpe].no_execute = protection_noexec;
            // Clear unspecified flags
            pdpe[va.pdpe].pwt = 0;
            pdpe[va.pdpe].pcd = 0;
            pdpe[va.pdpe].accesed = 0;
            pdpe[va.pdpe].ignored = 0;
            pdpe[va.pdpe].mbz = 0;
            pdpe[va.pdpe].ignored2 = 0;
            pdpe[va.pdpe].ats0 = 0;
            pdpe[va.pdpe].ats1 = 0;

            pde[va.pde].present = map_present;
            pde[va.pde].rw = protection_rw;
            pde[va.pde].user = !protection_supervisor;
            pde[va.pde].no_execute = protection_noexec;
            // Clear unspecified flags
            pde[va.pde].pwt = 0;
            pde[va.pde].pcd = 0;
            pde[va.pde].accesed = 0;
            pde[va.pde].ignored = 0;
            pde[va.pde].mbz = 0;
            pde[va.pde].ignored2 = 0;
            pde[va.pde].ats0 = 0;
            pde[va.pde].ats1 = 0;

            pte[va.pte].present = map_present;
            pte[va.pte].rw = protection_rw;
            pte[va.pte].user = !protection_supervisor;
            pte[va.pte].papn_ppn = physAddr >> 12;
            pte[va.pte].no_execute = protection_noexec;
            pte[va.pte].global = map_global;
            // Clear unspecified flags
            pte[va.pte].pwt = 0;
            pte[va.pte].pcd = 0;
            pte[va.pte].accesed = 0;
            pte[va.pte].dirty = 0;
            pte[va.pte].pat = 0;
            pte[va.pte].ats0 = 0;
            pte[va.pte].ats1 = 0;
            pte[va.pte].pkeys = 0;

            io::invalCache((void*)virtAddr);
            return true;
        }
        uint64_t allocateVmm(std::size_t size){
            VmFreelist* prev = nullptr;
            VmFreelist* current = freelist_head;

            while (current != nullptr)
            {
                // Calculate the aligned base address within the current block
                uint64_t aligned_base_address = (current->base_address + (4096 - 1)) & ~(4096 - 1);

                // Calculate the end address of the requested block
                uint64_t end_address = aligned_base_address + size;

                if (end_address <= current->base_address + current->size)
                {
                    // Update the current freelist entry
                    if (aligned_base_address == current->base_address)
                    {
                        // If allocation starts at the base of the current block
                        current->base_address += size;
                        current->size -= size;
                    }
                    else
                    {
                        // Split the current block
                        VmFreelist* new_entry = (VmFreelist*) MmSaAlloc(sizeof(VmFreelist));
                        new_entry->base_address = end_address;
                        new_entry->size = current->size - (end_address - current->base_address);
                        new_entry->next = current->next;

                        current->size = aligned_base_address - current->base_address;
                        current->next = new_entry;
                    }

                    if (current->size == 0)
                    {
                        // Remove the current block if it is fully allocated
                        if (prev != nullptr)
                        {
                            prev->next = current->next;
                        }
                        else
                        {
                            freelist_head = current->next;
                        }

                        MmSaFree(current, sizeof(VmFreelist));
                    }

                    return aligned_base_address;
                }

                prev = current;
                current = current->next;
            }

            // No suitable block found
            std::error("No suitable block found for allocation\n");
        }
    }