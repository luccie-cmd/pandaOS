#include <math>
#include <cstdint>
#include <cstring>

namespace std{
    double fabs(double number){
        // Use reinterpret_cast to treat the double's bit pattern as a uint64_t
        uint64_t bits;
        std::memcpy(&bits, &number, sizeof(bits));

        // Clear the sign bit (most significant bit of the 64-bit representation)
        bits &= 0x7FFFFFFFFFFFFFFF;

        // Convert back to double
        double result;
        std::memcpy(&result, &bits, sizeof(result));

        return result;
    }
}