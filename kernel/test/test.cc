#include "test.h"
#include <cstdio>
#include <dbg/dbg.h>
#include <fb/fb.h>
#include <mmu/mmu.h>
#include <cstring>

namespace test{
    void runAll(){
        std::pair<const char*, bool> DbgResult = runDBG();
        if(DbgResult.second == false){
            std::printf("Error message: %s\n", DbgResult.first);
            std::error("Failed to run Dbg test\n");
        }
        std::pair<const char*, bool> FbResult = runFB();
        if(FbResult.second == false){
            std::printf("Error message: %s\n", FbResult.first);            
            std::error("Failed to runFb test\n ");
        }
        std::pair<const char*, bool> MmuResult = runMMU();
        if(MmuResult.second == false){
            std::printf("Error message: %s\n", MmuResult.first);
            std::error("Failed to run Mmu test\n");
        }
        std::pair<const char*, bool> AcpiResult = runACPI();
        if(AcpiResult.second == false){
            std::printf("Error message: %s\n", AcpiResult.first);
            std::error("Failed to run Acpi test\n");
        }
        std::pair<const char*, bool> IrqResult = runIRQ();
        if(IrqResult.second == false){
            std::printf("Error message: %s\n", IrqResult.first);
            std::error("Failed to run Irq test\n");
        }
        std::pair<const char*, bool> VfsResult = runVFS();
        if(VfsResult.second == false){
            std::printf("Error message: %s\n", VfsResult.first);
            std::error("Failed to run Vfs test\n");
        }
        std::pair<const char*, bool> PciResult = runPCI();
        if(PciResult.second == false){
            std::printf("Error message: %s\n", PciResult.first);
            std::error("Failed to run Pci test\n");
        }
        std::pair<const char*, bool> SyscallResult = runSYSCALL();
        if(SyscallResult.second == false){
            std::printf("Error message: %s\n", SyscallResult.first);
            std::error("Failed to run Syscall test\n");
        }
        std::pair<const char*, bool> TssResult = runTSS();
        if(TssResult.second == false){
            std::printf("Error message: %s\n", TssResult.first);
            std::error("Failed to run Tss test\n");
        }
    }

    std::pair<const char*, bool> runDBG(){
        dbg::printf("Test 123\n");
        return {nullptr, true};
    }
    std::pair<const char*, bool> runFB(){
        framebuffer::setPixel(0, 0, 255, 0, 0);
        return {nullptr, true};
    }
    std::pair<const char*, bool> runMMU(){
        uint64_t physAddr1 = mmu::allocatePage();
        uint64_t physAddr2 = mmu::allocatePage();
        if(physAddr1 == physAddr2){
            return {"Allocate page same result", false};
        }
        mmu::freePage(physAddr2);
        if(!mmu::mapMemory(mmu::getPML4(), 0x401000, physAddr1, PROTECTION_NO_EXECUTE | PROTECTION_KERNEL | PROTECTION_RW, MAP_GLOBAL | MAP_PRESENT)){
            return {"Failed to map memory to 0x401000", false};
        }
        mmu::freePage(physAddr1);
        void* addr1 = mmu::allocateMemory(1024);
        void* addr2 = mmu::allocateMemory(4096);
        void* addr3 = mmu::allocateMemory(9216);
        if(addr1 == addr2 || addr2 == addr3 || addr1 == addr3){
            return {"Failed to allocate heap", false};
        }
        if(addr1 == nullptr || addr2 == nullptr || addr3 == nullptr){
            return {"Heap failed to return a valid address", false};
        }
        std::memcpy(addr1, "Hello\0", strlen("Hello")+1);
        mmu::freeMemory(addr2);
        mmu::freeMemory(addr1);
        mmu::freeMemory(addr3);
        return {nullptr, true};
    }
    std::pair<const char*, bool> runACPI(){
        return {nullptr, true};
    }
    std::pair<const char*, bool> runIRQ(){
        return {nullptr, true};
    }
    std::pair<const char*, bool> runVFS(){
        return {nullptr, true};
    }
    std::pair<const char*, bool> runPCI(){
        return {nullptr, true};
    }
    std::pair<const char*, bool> runSYSCALL(){
        return {nullptr, true};
    }
    std::pair<const char*, bool> runTSS(){
        return {nullptr, true};
    }
};