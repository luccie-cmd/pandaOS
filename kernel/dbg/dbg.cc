#include "dbg.h"
#include <io/io.h>
#include <cstdarg>
#include <cstring>
#include <stb_sprintf.h>

namespace dbg{
    void putc(char c){
        io::outb(0xE9, c);
    }

    void print(const char* str){
        while(*str){
            putc(*str);
            str++;
        }
    }
    void printf(const char* fmt, ...){
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        stbsp_vsnprintf(buffer, sizeof(buffer), fmt, args);
        print(buffer);
        va_end(args);
    }
}