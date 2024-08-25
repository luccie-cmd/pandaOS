#include <cstring>

namespace std{
    void* memcpy(void* dst, const void* src, std::size_t num){
        char* u8Dst = (char *)dst;
        const char* u8Src = (const char *)src;

        for (std::size_t i = 0; i < num; i++)
            u8Dst[i] = u8Src[i];

        return dst;
    }

    void* memset(void * ptr, int value, std::size_t num){
        char* u8Ptr = (char *)ptr;

        for (std::size_t i = 0; i < num; i++)
            u8Ptr[i] = (char)value;

        return ptr;
    }
    int strcmp(const char* a, const char* b){
        if (a == NULL && b == NULL)
            return 0;

        if (a == NULL || b == NULL)
            return -1;

        while (*a && *b && *a == *b){
            ++a;
            ++b;
        }
        return (*a) - (*b);
    }
    std::size_t strlen(const char *str){
        std::size_t len = 0;
        while(*str){
            len++;
            str++;
        }
        return len;
    }
    char *strncat(char *dest, const char *src, std::size_t n){
        char *d = dest;
        while (*d != '\0')
            d++;

        while (n-- && (*src != '\0'))
            *d++ = *src++;

        *d = '\0';
        return dest;
    }
    int memcmp(const void *ptr1, const void *ptr2, std::size_t count){
        const unsigned char *p1 = (const unsigned char *) ptr1;
        const unsigned char *p2 = (const unsigned char *) ptr2;
        while (count--){
            if (*p1 != *p2){
                return *p1 - *p2;
            }
            ++p1;
            ++p2;
        }
        return 0;
    }
};

extern "C" {
    void* memcpy(void* dest, const void* src, size_t count){
        return std::memcpy(dest, src, count);
    }
    void* memset(void* dest, int ch, size_t count){
        return std::memset(dest, ch, count);
    }
    int strcmp(const char* str1, const char* str2){
        return std::strcmp(str1, str2);
    }
    size_t strlen(const char *str){
        return std::strlen(str);
    }
    char *strncat(char *dest, const char *src, size_t n){
        return std::strncat(dest, src, n);
    }
    int memcmp(const void *ptr1, const void *ptr2, size_t count){
        return std::memcmp(ptr1, ptr2, count);
    }
}