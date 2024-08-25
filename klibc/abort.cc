#include <abort>

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
}