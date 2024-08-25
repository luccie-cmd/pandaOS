#include "dbg.h"
#include <io/io.h>

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
}