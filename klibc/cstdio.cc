#include <cstdio>
#include <cstring>
#include "stb_sprintf.h"

namespace framebuffer{
    extern void putc(char c);
}
namespace dbg{
    extern void putc(char c);
    extern void print(const char* str);
}

void* stderr = nullptr;

namespace std{
    void putc(char c){
        framebuffer::putc(c);
        dbg::putc(c);
    }
    void puts(const char *str){
        while(*str){
            putc(*str);
            str++;
        }
    }
    void vprintf(const char* fmt, va_list args){
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
        puts(buffer);
    }
    void printf(const char* fmt, ...){
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    void fprintf(void* stream, const char* fmt, ...){
        (void)stream;
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    [[noreturn]] void error(const char* fmt, ...){
        printf("ERROR: ");
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        while(1){}
    }
}

extern "C" {
    int fprintf(void* stream, const char* fmt, ...){
        (void)stream;
        va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
        return 0;
    }
}