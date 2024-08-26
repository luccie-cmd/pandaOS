#include "acpi.h"
#include <../limine/limine.h>
#include <cstdio>
#include <mmu/mmu.h>
#include <dbg/dbg.h>
#include <cstring>
#include <io/io.h>

namespace acpi{
    limine_rsdp_request rsdp_request = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
        .response = nullptr,
    };
    void* rsdp_addr = nullptr;
    XSDP* xsdp = nullptr;
    XSDT* xsdt = nullptr;
    std::size_t numEntries = 0;

    bool supportsXSDT(XSDP* xsdp){
        return (xsdp->Revision > 0);
    }
    bool doChecksum(ACPISDTHeader *tableHeader){
        unsigned char sum = 0;

        for (uint32_t i = 0; i < tableHeader->Length; i++){
            sum += ((char *) tableHeader)[i];
        }

        return sum == 0;
    }
    void* getTableBySign(char sign[4]){
        std::printf("\tLoading %.4s Table\n", sign);
        for(std::size_t i = 0; i < numEntries; ++i){
            ACPISDTHeader *sdt = (ACPISDTHeader*)(xsdt->Entries[i]);
            if(std::memcmp(sdt->Signature, sign, 4) == 0){
                return (void*)sdt;
            }
        }
        std::error("No table with sign %.4s present\n", sign);
    }
    void initFADT(){
        std::printf("\tSetting up ACPI mode\n");
        char fadt_sign[4] = {'F', 'A', 'C', 'P'};
        auto table = (FADT*) getTableBySign(fadt_sign);
        if (table->SMI_CommandPort == 0){
            dbg::print("ACPI mode already enabled. SMI command port == 0.\n");
            return;
        }

        if ((table->AcpiEnable == table->AcpiDisable) == 0){
            dbg::print("ACPI mode already enabled. table->AcpiEnable == table->AcpiDisable == 0\n");
            return;
        }

        if (table->PM1aControlBlock & 1){
            dbg::print("ACPI mode already enabled. (table->PM1aControlBlock & 1) == 1\n");
            return;
        }

        io::outb(table->SMI_CommandPort, table->AcpiEnable);
        while ((io::inw(table->PM1aControlBlock) & 1) == 0)
        {
            asm volatile ("nop");
        }
    }
    void init(){
        std::printf("ACPI:\n");
        if(rsdp_request.response == nullptr){
            std::error("\tNULL RSDP (Root System Descriptor Pointer) request\n");
        }
        rsdp_addr = rsdp_request.response->address;
        xsdp = (XSDP*)(rsdp_addr);
        if(!supportsXSDT(xsdp)){
            std::error("\tMust support ACPI XSDT\n");
        }
        xsdt = (XSDT*)(xsdp->XsdtAddress+mmu::getHHDM());
        if(!doChecksum(&xsdt->h)){
            std::error("\tInvalid XSDT chechsum\n");
        }
        std::printf("\tLoaded XSDT\n");
        numEntries = (xsdt->h.Length - sizeof(xsdt->h)) / sizeof(uint64_t);
        initFADT();
    }
    void findMADT(::std::function<void(void*)> lambda){
        char madt_sign[4] = {'A', 'P', 'I', 'C'};
        auto table = (MADT*)getTableBySign(madt_sign);
        uint8_t* entries_start = reinterpret_cast<uint8_t*>(table) + sizeof(MADT);
        uint8_t* entries_end = reinterpret_cast<uint8_t*>(table) + table->sdt.Length;

        uint8_t* current_entry = entries_start;
        while (current_entry < entries_end)
        {
            MADTEntry* entry = reinterpret_cast<MADTEntry*>(current_entry);
            lambda((void*)entry);
            current_entry += entry->entry_length;
        }
    }
    void printInfo(){
        std::printf("\txsdp = 0x%lx\n", xsdp);
        std::printf("\txsdt = 0x%lx\n", xsdt);
        std::printf("\tnums = 0x%lx\n", numEntries);
        for (std::size_t i = 0; i < numEntries; ++i) {
            ACPISDTHeader *sdt = (ACPISDTHeader*)(xsdt->Entries[i]);
            std::printf("\tEntry %zu: 0x%lx Signature: %.4s\n", i, sdt, sdt->Signature);
        }
    }
}