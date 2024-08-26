#include "irq.h"
#include <cstdio>

namespace hal::irq{
    void init(){
        std::printf("IRQ:\n");
        initAPIC();
        acpi::findMADT([](void* madt_entry_ptr){
            acpi::MADTEntry* entry_header = (acpi::MADTEntry*)(madt_entry_ptr);
            switch(entry_header->entry_type){
                case MADT_ENTRY_TYPE_LAPIC_NON_MASKABLE_INTERRUPTS:
                case MADT_ENTRY_TYPE_LAPIC: {} break;
                case MADT_ENTRY_TYPE_IOAPIC: {
                    acpi::MADTIOAPIC* ioapic = (acpi::MADTIOAPIC*)madt_entry_ptr;
                    initIOAPIC(ioapic);
                } break;
                case MADT_ENTRY_TYPE_IOAPIC_INTERRUPT_SOURCE_OVERRIDE: {
                    acpi::MADTIOAPICISO *isoapic = (acpi::MADTIOAPICISO*)madt_entry_ptr;
                    handleIOAPICISO(isoapic);
                } break;
                default: {
                    std::error("Unhandled MADT entry type %d\n", entry_header->entry_type);
                } break;
            }
        });
    }
}