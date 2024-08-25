#pragma once
#include <cstdint>
#include <../limine/limine.h>

namespace mmu{
    #define PAGE_SIZE 4096

    #define PROTECTION_RW           (1 << 0)
    #define PROTECTION_KERNEL       (1 << 1)
    #define PROTECTION_USER         (1 << 2)
    #define PROTECTION_NO_EXECUTE   (1 << 3)
    #define PROTECTION_NONE         (0)

    #define MAP_NONE        (0)
    #define MAP_PRESENT     (1 << 0)
    #define MAP_GLOBAL      (1 << 1)

    struct pml4
    {
        uint64_t present    : 1; // Is the page present?
        uint64_t rw         : 1; // Can read/write to/from the page?
        uint64_t user       : 1; // Can user access this page?
        uint64_t pwt        : 1; // Page write through
        uint64_t pcd        : 1; // Page cache disable
        uint64_t accesed    : 1; // Is page accessed?
        uint64_t ignored    : 1; // Ignored field.
        uint64_t mbz        : 2; // Must be zero.
        uint64_t ats0       : 3; // Available to software.
        uint64_t pdpe_ptr   : 40; // Physical page number to the PDP tables.
        uint64_t ats1       : 11; // Available to the software.
        uint64_t no_execute : 1;  // Disable execution of code on this page.
    } __attribute__((packed));
    static_assert(sizeof(pml4) == 8, "Structure PML4 isn't 8 bytes big.");

    struct pdpe
    {
        uint64_t present    : 1; // Is the page present?
        uint64_t rw         : 1; // Can read/write to/from the page?
        uint64_t user       : 1; // Can user access this page?
        uint64_t pwt        : 1; // Page write through
        uint64_t pcd        : 1; // Page cache disable
        uint64_t accesed    : 1; // Is page accessed?
        uint64_t ignored    : 1; // Ignored field.
        uint64_t mbz        : 1; // Must be zero.
        uint64_t ignored2   : 1; // Ignored field.
        uint64_t ats0       : 3; // Available to software.
        uint64_t pde_ptr    : 40; // Physical page number to the PD tables.
        uint64_t ats1       : 11; // Available to the software.
        uint64_t no_execute : 1;  // Disable execution of code on this page.
    } __attribute__((packed));
    static_assert(sizeof(pdpe) == 8, "Structure PDP isn't 8 bytes big.");

    struct pde
    {
        uint64_t present    : 1; // Is the page present?
        uint64_t rw         : 1; // Can read/write to/from the page?
        uint64_t user       : 1; // Can user access this page?
        uint64_t pwt        : 1; // Page write through
        uint64_t pcd        : 1; // Page cache disable
        uint64_t accesed    : 1; // Is page accessed?
        uint64_t ignored    : 1; // Ignored field.
        uint64_t mbz        : 1; // Must be zero.
        uint64_t ignored2   : 1; // Ignored field.
        uint64_t ats0       : 3; // Available to software.
        uint64_t pte_ptr    : 40; // Physical page number to the PT tables.
        uint64_t ats1       : 11; // Available to the software.
        uint64_t no_execute : 1;  // Disable execution of code on this page.
    } __attribute__((packed));
    static_assert(sizeof(pde) == 8, "Structure PD isn't 8 bytes big.");

    struct pte
    {
        uint64_t present    : 1; // Is the page present?
        uint64_t rw         : 1; // Can read/write to/from the page?
        uint64_t user       : 1; // Can user access this page?
        uint64_t pwt        : 1; // Page write through
        uint64_t pcd        : 1; // Page cache disable
        uint64_t accesed    : 1; // Is page accessed?
        uint64_t dirty      : 1; // Was the field written to?
        uint64_t pat        : 1; // Page attribute table.
        uint64_t global     : 1; // Is page global (unvalidated)
        uint64_t ats0       : 3; // Available to software.
        uint64_t papn_ppn   : 40; // Physical page number to the physical address.
        uint64_t ats1       : 7;  // Available to the software.
        uint64_t pkeys      : 4;  // Protection keys
        uint64_t no_execute : 1;  // Disable execution of code on this page.
    } __attribute__((packed));
    static_assert(sizeof(pte) == 8, "Structure PT isn't 8 bytes big.");

    void init();
    void init_mm();
    void init_heap(std::size_t p_pmm_size, std::size_t p_vmm_max);
    void printInfo();
    void printVmmInfo();
    void printPmmInfo();
    void printHeapInfo();
    uint64_t makeVirtual(uint64_t addr);
    uint64_t getHHDM();
    uint64_t allocatePage();
    void freePage(uint64_t addr);
    pml4* getPML4();
    bool mapMemory(pml4* pml4e, uint64_t virtAddr, uint64_t physAddr, int prot, int flags);
    void* allocateMemory(std::size_t size);
    void freeMemoryHeap(void* ptr);
    void heapExtend(std::size_t pages);
    void heapCoalesceBlocks();
    uint64_t allocateVmm(std::size_t size);
    limine_memmap_entry** retrieveMemoryMap();
    uint64_t retrieveMemoryMapEntryCount();
};