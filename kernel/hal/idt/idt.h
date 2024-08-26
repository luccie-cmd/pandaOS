#pragma once
#include <cstdint>
#include <io/io.h>

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

    #define IDT_GATE_TYPE_INTERRUPT 0xE
    #define IDT_GATE_TYPE_TRAP_GATE 0xF
    
    #define IDT_ENTRY(offset, segment, type, dpl, ist) {                     \
        (uint16_t)((offset) & 0xFFFF),                 /* offset0 */        \
        (segment),                                     /* segment_sel */    \
        (ist),                                         /* ist */            \
        0,                                             /* reserved0 */      \
        (type),                                        /* gate_type */      \
        0,                                             /* zero */           \
        (dpl),                                         /* dpl */            \
        1,                                             /* present */        \
        (uint16_t)(((offset) >> 16) & 0xFFFF),         /* offset1 */        \
        (uint32_t)(((offset) >> 32) & 0xFFFFFFFF),     /* offset2 */        \
        0                                              /* reserved1 */      \
    }

    struct IDT{
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));

    void init();
    void enableGate(uint8_t idx);
    void initGates();
    void registerHandler(uint8_t idx, void* function, uint8_t type);
    extern "C" void handleInt(io::Registers* regs);
};
};