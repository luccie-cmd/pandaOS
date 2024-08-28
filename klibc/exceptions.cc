#include <cstdio>

namespace io{
    extern void halt();
}

namespace std{
    void __throw_bad_function_call(){
        // TODO: Diag print trace
        std::printf("Bad function call\n");
        io::halt();
    }
    void __throw_bad_alloc(){
        std::printf("Bad allocation\n");
        io::halt();
    }
    void __throw_bad_array_new_length(){
        std::printf("Bad new array length\n");
        io::halt();
    }
    void __throw_length_error(const char* e){
        std::printf("Bad length: ");
        std::printf("%s\n", e);
        io::halt();
    }
    void __throw_out_of_range_fmt(const char* e, ...){
        std::printf("Out of range: ");
        va_list args;
        va_start(args, e);
        std::vprintf(e, args);
        va_end(args);
        std::printf("\n");
        io::halt();
    }
};