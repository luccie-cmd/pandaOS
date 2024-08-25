#include "io.h"
#include <fb/fb.h>
#include <cstdio>

namespace io{
    void outb(std::uint16_t port, std::uint8_t value){
        __asm__ volatile (
            "outb %b0, %w1"
            :
            :
            "a"(value), "Nd"(port)
            : "memory"
            );
    }

    uint64_t cr3(){
        uint64_t cr3;
        __asm__ volatile (
            "mov %%cr3, %0" 
            : "=r"(cr3)
            );
        return cr3;
    }

    void invalCache(void* page){
        __asm__ volatile (
            "invlpg (%0)"
            :
            : "r" (page)
            : "memory"
            );
    }

    void panic(const char* str){
        std::error("System critical error: %s\n", str);
    }
}