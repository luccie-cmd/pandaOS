#include "irq.h"
#include <cstdio>
#include <mmu/mmu.h>

namespace hal::irq{
    uint64_t ioapic_base = 0;
    // Function to write a 32-bit register from the I/O APIC
    void ioapic_write(uint32_t reg, uint32_t value) {
        volatile uint32_t* addr = (volatile uint32_t*)(ioapic_base + 0x00);
        volatile uint32_t* data = (volatile uint32_t*)(ioapic_base + 0x10);
        
        // Write the register index to the IOREGSEL register
        *addr = reg;
        // Write the value to the IOWIN register
        *data = value;
    }

    // Function to read a 32-bit value from an I/O APIC register
    uint32_t ioapic_read(uint32_t reg) {
        volatile uint32_t* addr = (volatile uint32_t*)(ioapic_base + 0x00);
        volatile uint32_t* data = (volatile uint32_t*)(ioapic_base + 0x10);

        // Write the register index to the IOREGSEL register
        *addr = reg;
        // Read the value from the IOWIN register
        return *data;
    }

    // Function to initialize the I/O APIC
    void initIOAPIC(acpi::MADTIOAPIC* ioapic) {
        ioapic_base = (uint64_t)ioapic->ioapic_address + mmu::getHHDM();
        uint32_t ioapic_id = ioapic_read(0x00) >> 24;

        // Print IOAPIC ID to verify that we're accessing the correct IOAPIC
        std::printf("\tInitializing I/O APIC ID: %u\n", ioapic_id);

        // Set up redirection entries in the I/O APIC's redirection table
        for (uint32_t i = 0; i < 24; ++i) {  // Typically, there are 24 entries, but this can vary
            uint32_t low_index = 0x10 + 2 * i;  // Low part of the redirection entry
            uint32_t high_index = 0x10 + 2 * i + 1;  // High part of the redirection entry

            // Example configuration:
            // - IRQ vector = 0x20 + i
            // - Delivery mode = Fixed
            // - Destination CPU = 0
            ioapic_write(low_index, 0x00010020 + i);
            ioapic_write(high_index, 0x00000000);
        }
    }
    void handleIOAPICISO(acpi::MADTIOAPICISO *ioapiciso){
        // Calculate the redirection entry index based on the GSI
        uint32_t redirection_entry = ioapiciso->gsi;

        // IOAPIC Redirection Entry registers: 
        //   - Low 32 bits: register = 0x10 + 2 * entry index
        //   - High 32 bits: register = 0x10 + 2 * entry index + 1
        uint32_t low_index = 0x10 + 2 * redirection_entry;
        uint32_t high_index = low_index + 1;

        // Read the current redirection table entry
        uint32_t low_value = ioapic_read(low_index);
        uint32_t high_value = ioapic_read(high_index);

        // Modify the redirection entry to map SourceIRQ to the specified GSI
        // For example, we might set the delivery mode, vector, and mask fields
        low_value &= ~0xFF;  // Clear vector bits
        low_value |= ioapiciso->irq_source;  // Set vector bits to the SourceIRQ

        // Update the trigger mode and polarity if specified by the flags
        if (ioapiciso->flags & 0x2) {
            // Set level-triggered (instead of edge-triggered)
            low_value |= (1 << 15);
        } else {
            // Set edge-triggered
            low_value &= ~(1 << 15);
        }

        if (ioapiciso->flags & 0x8) {
            // Set active-low polarity
            low_value |= (1 << 13);
        } else {
            // Set active-high polarity
            low_value &= ~(1 << 13);
        }

        // Write the modified values back to the IOAPIC redirection table
        ioapic_write(low_index, low_value);
        ioapic_write(high_index, high_value);

        std::printf("\tIOAPIC redirection entry %u set to 0x%x:0x%x\n", 
                    redirection_entry, high_value, low_value);
    }
}