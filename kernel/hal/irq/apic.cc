#include "irq.h"
#include <mmu/mmu.h>
#include <io/io.h>
#include <cstdio>

namespace hal::irq{
    uint64_t apic_base;
    void apicWrite(uint64_t offset, uint32_t value){
        volatile uint32_t* addr = (volatile uint32_t*) (apic_base + offset);
        *addr = value;
    }
    void APICEoi(){
        apicWrite(LAPIC_EOI_REGISTER, 0);
    }
    void initAPIC(){
        apic_base = io::rdmsr(IA32_APIC_BASE_MSR);
        apic_base &= ~(0xfff);
        apic_base += mmu::getHHDM();
        apicWrite(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1FF);
        apicWrite(LAPIC_TASK_PRIORITY_REGISTER, 0); // Allow them all.
        // apicWrite(LAPIC_LVT_TIMER_REGISTER, 0x20 | (1 << 17)); // Allow timer
        std::printf("  APIC:\n\tBase: %lx\n", apic_base);
    }
};