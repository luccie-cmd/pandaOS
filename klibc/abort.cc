#include <abort>
#include <cstdio>

namespace io{
    [[noreturn]] void halt();
}

namespace std{
    [[noreturn]] void abort(void) noexcept {
        io::halt();
    }
}

extern "C" {
    void abort(void) {
        std::abort();
    }
    [[noreturn]] void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function){
        std::printf("[%s %d %s] Assertion failed: %s\n", file, line, function, assertion);
        io::halt();
    }
}