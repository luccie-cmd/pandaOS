#include <cstdio>

namespace io{
    void halt();
}

namespace std{
    void __throw_bad_function_call(){
        // TODO: Diag print trace
        std::printf("Bad function call\n");
        io::halt();
    }
};