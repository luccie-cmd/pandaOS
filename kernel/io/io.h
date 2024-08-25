#pragma once
#include <cstdint>

namespace io{
    void outb(std::uint16_t port, std::uint8_t value);
    uint64_t cr3();
    void invalCache(void* addr);
    void panic(const char* str);
    void cli();
    void sti();
    void hlt();
    [[noreturn]] void halt();
}