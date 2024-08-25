#include <cstring>

namespace std{
    void* memcpy(void* dst, const void* src, std::size_t num){
        char* u8Dst = (char *)dst;
        const char* u8Src = (const char *)src;

        for (std::size_t i = 0; i < num; i++)
            u8Dst[i] = u8Src[i];

        return dst;
    }

    void * memset(void * ptr, int value, std::size_t num){
        char* u8Ptr = (char *)ptr;

        for (std::size_t i = 0; i < num; i++)
            u8Ptr[i] = (char)value;

        return ptr;
    }
};