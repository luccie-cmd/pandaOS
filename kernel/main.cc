#include <fb/fb.h>
#include <dbg/dbg.h>
#include <mmu/mmu.h>
#include <io/io.h>
#include <cstdio>
#include <hal/gdt/gdt.h>
#include <hal/idt/idt.h>
#include <cstring>
#include <acpi/acpi.h>

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
    dbg::print("Booted OS\n");
    framebuffer::init();
    framebuffer::printInfo();
    mmu::init();
    mmu::printInfo();
    hal::gdt::init();
    hal::idt::init();
    acpi::init();
    acpi::printInfo();
    std::error("Implement IRQ\n"); // ACPI's MADT (multiple APIC descriptor table) needs to be read first before initializing the IRQ
    std::error("Implement PCI\n");
    std::error("Implement VFS\n");
    AbiCallCtors();
    std::error("Implement SYSCALLS\n");
    std::error("Implement TSS\n");
    for(;;);
}