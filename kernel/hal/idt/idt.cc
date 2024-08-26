#include "idt.h"
#include <cstdio>
#include <cassert>
extern "C"
{
#include <xed/xed-interface.h>
}
#include <dbg/decomp/decompiler.h>
#include <hal/irq/irq.h>

hal::idt::IDTEntry idtEntries[256];

static const char* const exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

extern "C" void loadIDT(hal::idt::IDT* idt);

namespace hal::idt{
    void initGates();
    void init(){
        IDT idt = {
            .limit = sizeof(idtEntries)-1,
            .base = (uint64_t)idtEntries
        };
        loadIDT(&idt);
        std::printf("IDT:\n\tLoaded new IDT\n");

        initGates();
        for(int i = 0; i < 256; ++i){
            enableGate(i);
        }
        std::printf("\tLoaded ISR\n");
    }
    void registerHandler(uint8_t idx, void* function, uint8_t type){
        idtEntries[idx] = IDT_ENTRY((uint64_t)function, 0x8, type, 0, 0);
    }
    void enableGate(uint8_t idx){
        idtEntries[idx].present = 1;
    }
    extern "C" void handleInt(io::Registers* regs){
        if(regs->interrupt_number < 32){
            xed_decoded_inst_t xedd;
            DecompilerDecompileAtRIPRange(&xedd, (const uint8_t*)regs->rip, 32, 32);
            std::error("%s\n", exceptions[regs->interrupt_number]);
        }
        std::error("TODO: Handle int %ld with code %ld\n", regs->interrupt_number, regs->error_code);
    }
};