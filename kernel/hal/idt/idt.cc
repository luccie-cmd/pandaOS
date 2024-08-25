#include "idt.h"
#include <cstdio>

hal::idt::IDTEntry idtEntries[256];

extern "C" void loadIDT(hal::idt::IDT* idt);

namespace hal::idt{
    void init(){
        IDT idt = {
            .limit = sizeof(idtEntries)-1,
            .base = (uint64_t)idtEntries
        };
        loadIDT(&idt);
        std::printf("IDT:\n\tLoaded new IDT\n");
    }
};