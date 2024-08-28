#include "mmu.h"
#include <cstdio>
#include <dbg/dbg.h>
#include <io/io.h>

namespace mmu{
    struct alignas(16) HeapBlock
    {
        std::size_t length;
        bool is_allocated;
        HeapBlock *next, *prev;
    };
    HeapBlock* freeList = nullptr;
    std::size_t pmm_size;
    std::size_t vmm_max;
    void* vmm_base = nullptr;

    void init_heap(std::size_t p_pmm_size, std::size_t p_vmm_max){
        pmm_size = p_pmm_size;
        vmm_max = p_vmm_max;
        if (pmm_size > vmm_max || (pmm_size % PAGE_SIZE) != 0)
        {
            std::error("Invalid memory parameters.\n");
        }
        vmm_base = (void*)allocateVirtualMemory(vmm_max);
        if (!vmm_base)
        {
            std::error("Failed to allocate virtual memory.\n");
        }
        // Allocate the inital physical memory
        for(std::size_t pg_off = 0; pg_off < pmm_size; pg_off += PAGE_SIZE){
            uint64_t physAddr = allocatePage();
            if(physAddr == 0){
                std::error("physical address was 0\n");
            }
            if(!mapMemory(getPML4(), reinterpret_cast<uint64_t>(vmm_base)+pg_off, physAddr, PROTECTION_KERNEL | PROTECTION_RW, MAP_GLOBAL | MAP_PRESENT)){
                std::error("Failed to map memory\n");
            }
        }

        freeList = reinterpret_cast<HeapBlock*>(vmm_base);
        freeList->length = pmm_size - sizeof(HeapBlock);
        freeList->is_allocated = false;
        freeList->next = nullptr;
        freeList->prev = nullptr;
    }
    void printHeapInfo(){
        std::printf("  HEAP:\n");
        std::printf("\tPhysical memory current size = 0x%lx\n", pmm_size);
        std::printf("\tVirtual memory max           = 0x%lx\n", vmm_max);
        std::printf("\tVirtual memory base          = 0x%lx\n", vmm_base);
    }
    void* allocateMemory(std::size_t size){
        std::size_t allignedLength = (size + 16 - 1) & ~(16 - 1);

        HeapBlock* current = freeList;
        while(current){
            if(!current->is_allocated && current->length >= allignedLength){
                if(current->length > allignedLength + sizeof(HeapBlock)){
                    // Split into 2
                    HeapBlock* new_block = reinterpret_cast<HeapBlock*>(
                        reinterpret_cast<uint8_t*>(current) + sizeof(HeapBlock) + allignedLength
                    );
                    new_block->length = current->length - allignedLength - sizeof(HeapBlock);
                    new_block->is_allocated = false;
                    new_block->next = current->next;
                    new_block->prev = current;

                    if (current->next)
                    {
                        current->next->prev = new_block;
                    }
                    current->next = new_block;

                    current->length = allignedLength;
                }
                current->is_allocated = true;
                return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current)+sizeof(HeapBlock));
            }
            current = current->next;
        }
        dbg::print("No suitable block found, allocating 1 more page and trying again\n");
        heapExtend(1);
        return allocateMemory(allignedLength);
    }
    void freeMemory(void* ptr){
        if (!ptr) return;

        // Get the block associated with the pointer
        HeapBlock* block = reinterpret_cast<HeapBlock*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(HeapBlock));
        block->is_allocated = false;

        // Coalesce free blocks if possible
        heapCoalesceBlocks();
    }   
    void heapExtend(std::size_t pages){
        (void)pages;
        std::error("TODO Heap extend\n");
        // std::size_t new_pmem_size = pmm_size + pages * 4096;

        // if (new_pmem_size > vmm_max)
        // {
        //     dbg::printf("Heap out of virtual memory %d %llx > %llx\n", (uint32_t)pages, (new_pmem_size), (vmm_max));
        //     io::halt();
        // }

        // for (std::align_val_t pg_off = pmem_size; pg_off < new_pmem_size; pg_off += 4096)
        // {
        //     auto pg_paddr = MmAllocatePage();
        //     if (pg_paddr == 0)
        //     {
        //         DbgPrint("Failed to allocate physical page.\n");
        //         IoHaltProcessor();
        //     }

        //     auto status = MmMapPage(
        //             MmGetKernelPML4(),
        //             reinterpret_cast<uint64_t>(vmem_base) + pg_off,
        //             (uint64_t) pg_paddr,
        //             PROTECTION_KERNEL | PROTECTION_RW,
        //             MAP_GLOBAL | MAP_PRESENT
        //     );

        //     if (status != MAP_SUCCESS)
        //     {
        //         DbgPrint("Failed to map heap pages during extension.\n");
        //         IoHaltProcessor();
        //     }
        // }

        // pmem_size = new_pmem_size;

        // // Add the newly allocated memory to the free list
        // HeapBlock* new_block = reinterpret_cast<HeapBlock*>(
        //         reinterpret_cast<uint8_t*>(vmem_base) + pmem_size - pages * 4096
        // );

        // new_block->length = pages * 4096 - sizeof(HeapBlock);
        // new_block->is_allocated = false;
        // new_block->next = nullptr;

        // HeapBlock* current = free_list;
        // while (current->next)
        // {
        //     current = current->next;
        // }

        // current->next = new_block;
        // new_block->prev = current;

        // // Coalesce blocks after extension
        // heapCoalesceBlocks();
    }
    void heapCoalesceBlocks(){
        HeapBlock* current = freeList;
        while (current && current->next)
        {
            if (!current->is_allocated && !current->next->is_allocated)
            {
                current->length += sizeof(HeapBlock) + current->next->length;
                current->next = current->next->next;

                if (current->next)
                {
                    current->next->prev = current;
                }
            }
            else
            {
                current = current->next;
            }
        }
    }
}