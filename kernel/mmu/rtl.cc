#include "mmu.h"

void* operator new(std::size_t size)
{
    return mmu::allocateMemory(size);
}

void* operator new[](std::size_t size)
{
    return mmu::allocateMemory(size);
}

void operator delete(void* p) noexcept
{
    mmu::freeMemory(p);
}

void operator delete[](void* p) noexcept
{
    mmu::freeMemory(p);
}

void operator delete(void* p, std::size_t size){
    (void)size;
    mmu::freeMemory(p);
}

void operator delete[](void* p, std::size_t size){
    (void)size;
    mmu::freeMemory(p);
}