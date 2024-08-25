#include "mmu.h"
#include <cstdio>

#define GIGABYTE (1024 * 1024 * 1024ULL)
#define MEGABYTE (1024 * 1024ULL)

namespace mmu{
    void init(){
        init_mm();
        init_heap(1 * MEGABYTE, 1 * GIGABYTE);
    }
    void printInfo(){
        std::printf("Memory info:\n");
        printVmmInfo();
        printPmmInfo();
        printHeapInfo();
    }
};