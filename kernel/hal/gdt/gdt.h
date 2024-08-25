#pragma once
#include <cstdint>

namespace hal{
namespace gdt{
    struct GDTEntry{
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t limit1 : 4;
        uint8_t flags : 4;
        uint8_t base2 : 8;
    } __attribute__((packed));
    struct GDT{
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));
    void init();
};
};