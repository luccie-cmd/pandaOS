#pragma once
#include <cstdint>

namespace hal{
namespace idt{
    struct IDTEntry{
        uint16_t offset0;
        uint16_t segment_sel;
        uint8_t ist : 3;
        uint8_t reserved0 : 5;
        uint8_t gate_type : 4;
        uint8_t zero : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
        uint16_t offset1;
        uint32_t offset2;
        uint32_t reserved1;
    } __attribute__((packed));

    struct IDT{
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));
    void init();
};
};