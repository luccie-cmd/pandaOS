#pragma once
#include <cstdint>

namespace io{
    struct Registers{
        uint64_t cr3, gs, fs, es, ds, r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rsp, rdx, rcx, rbx, rax, interrupt_number, error_code, rip, cs, rflags, orig_rsp, ss;
    } __attribute__((packed));
    void outb(uint16_t port, uint8_t value);
    uint16_t inw(uint16_t port);
    uint64_t cr3();
    void invalCache(void* addr);
    void panic(const char* str);
    void cli();
    void sti();
    void hlt();
    [[noreturn]] void halt();
}