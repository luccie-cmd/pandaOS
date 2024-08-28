#include <fb/fb.h>
#include <dbg/dbg.h>
#include <mmu/mmu.h>
#include <io/io.h>
#include <cstdio>
#include <hal/gdt/gdt.h>
#include <hal/idt/idt.h>
#include <hal/irq/irq.h>
#include <acpi/acpi.h>
#include <test/test.h>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

void AbiCallCtors()
{
    for (std::size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }
}

extern "C" void KernelInit(){
    framebuffer::init();
    framebuffer::printInfo();
    mmu::init();
    mmu::printInfo();
    test::runMMU();
    hal::gdt::init();
    hal::idt::init();
    io::sti();
    acpi::init();
    acpi::printInfo();
    hal::irq::init();
    std::error("Implement VFS\n");
    std::error("Implement PCI\n");
    AbiCallCtors();
    std::error("Implement SYSCALLS\n"); 
    std::error("Implement TSS\n");
    test::runAll();
    for(;;);
}