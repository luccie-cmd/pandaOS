#pragma once
#include <cstdint>
#include <acpi/acpi.h>

namespace hal{
namespace irq{
    #define IA32_APIC_BASE_MSR 0x1B

    #define LAPIC_ID_REGISTER                                   0x20
    #define LAPIC_VERSION_REGISTER                              0x30
    #define LAPIC_TASK_PRIORITY_REGISTER                        0x80
    #define LAPIC_ARBITRATION_PRIORITY_REGISTER                 0x90
    #define LAPIC_PROCESSOR_PRIORITY_REGISTER                   0xA0
    #define LAPIC_EOI_REGISTER                                  0xB0
    #define LAPIC_REMOTE_READ_REGISTER                          0xC0
    #define LAPIC_LOGICAL_DESTINATION_REGISTER                  0xD0
    #define LAPIC_DESTINATION_FORMAT_REGISTER                   0xE0
    #define LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER            0xF0
    #define LAPIC_IN_SERVICE_REGISTER_REGISTER                  0x100
    #define LAPIC_TRIGGER_MODE_REGISTER                         0x180
    #define LAPIC_INTERRUPT_REQUEST_REGISTER                    0x200
    #define LAPIC_ERROR_STATUS_REGISTER                         0x280
    #define LAPIC_LVT_CMCI_REGISTER                             0x2F0 // LVT Corrected Machine Check Interrupt (CMCI) Register
    #define LAPIC_INTERRUPT_COMMAND_REGISTER                    0x300
    #define LAPIC_LVT_TIMER_REGISTER                            0x320
    #define LAPIC_LVT_THERMAL_SENSOR_REGISTER                   0x330
    #define LAPIC_LVT_PMC_REGISTER                              0x340 // LVT Performance Monitoring Counters Register
    #define LAPIC_LVT_LINT0_REGISTER                            0x350
    #define LAPIC_LVT_LINT1_REGISTER                            0x360
    #define LAPIC_LVT_ERROR_REGISTER                            0x370
    #define LAPIC_INITIAL_COUNT_REGISTER_TIMER_REGISTER         0x380
    #define LAPIC_CURRENT_COUNT_TIMER_REGISTER                  0x390
    #define LAPIC_DIVIDE_CONFIGURATION_REGISTER_TIMER_REGISTER  0x3E0

    void initIOAPIC(acpi::MADTIOAPIC *ioapic);
    void handleIOAPICISO(acpi::MADTIOAPICISO *ioapiciso);
    void APICEoi();
    void initAPIC();
    void init();
};
};