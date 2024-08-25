/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* We want the symbol _start to be our entry point */
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2)) ; /* Dynamic PHDR for relocations */
}

SECTIONS
{
    /* We wanna be placed in the topmost 2GiB of the address space, for optimizations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;

    /* .text section */
    .text ALIGN(4K) :
    {
        __text_start = .;
        *(.text .text.*)
        __text_end = .;
    } :text
    /* .rodata section */
    .rodata ALIGN(4K) :
    {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    } :rodata
    /* .data section */
    .data ALIGN(4K) :
    {
        __data_start = .;
        *(.data .data.*)
        KEEP(*(.gnu.linkonce.t.*)) /* Ensure vtable sections are kept */
        KEEP(*(.data.*)) /* Keep other data sections if necessary */
        __data_end = .;
    } :data
    /* Dynamic section for relocations, both in its own PHDR and inside data PHDR */
    .dynamic ALIGN(4K) :
    {
        __dynamic_start = .;
        *(.dynamic)
        __dynamic_end = .;
    } :data :dynamic
    /* Constructors */
    .init_array : {
        __init_array = .;
        *(.init_array .init_array.*)
        __init_array_end = .;
    } :rodata
    /* .bss section */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    } :data


    /DISCARD/ : { *(.note.gnu.build-id) *(.eh_frame*) }
}
