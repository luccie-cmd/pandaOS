#include <cstdio>
#include <math>

namespace framebuffer{
    extern void putc(char c);
}
namespace dbg{
    extern void putc(char c);
    extern void print(const char* str);
}

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

    #define PRINTF_STATE_NORMAL         0
    #define PRINTF_STATE_LENGTH         1
    #define PRINTF_STATE_LENGTH_SHORT   2
    #define PRINTF_STATE_LENGTH_LONG    3
    #define PRINTF_STATE_SPEC           4

    #define PRINTF_LENGTH_DEFAULT       0
    #define PRINTF_LENGTH_SHORT_SHORT   1
    #define PRINTF_LENGTH_SHORT         2
    #define PRINTF_LENGTH_LONG          3
    #define PRINTF_LENGTH_LONG_LONG     4

    const char g_HexChars[] = "0123456789abcdef";

    void printf_unsigned(unsigned long long number, int radix)
    {
        char buffer[32];
        int pos = 0;

        // convert number to ASCII
        do 
        {
            unsigned long long rem = number % radix;
            number /= radix;
            buffer[pos++] = g_HexChars[rem];
        } while (number > 0);

        // print number in reverse order
        while (--pos >= 0)
            putc(buffer[pos]);
    }

    void printf_signed(long long number, int radix)
    {
        if (number < 0)
        {
            putc('-');
            printf_unsigned(-number, radix);
        }
        else printf_unsigned(number, radix);
    }

    void printf_double(double d)
    {
        // Handle negative numbers
        if (d < 0) {
            putc('-');
            d = -d;
        }

        // Print integer part
        unsigned long long integerPart = static_cast<unsigned long long>(d);
        printf_unsigned(integerPart, 10);

        // Print decimal point
        putc('.');

        // Print fractional part
        const int maxPrecision = 16;
        int precision = 0;
        while (precision < maxPrecision)
        {
            d -= integerPart; // Subtract integer part
            d *= 10; // Move next decimal place
            integerPart = static_cast<unsigned int>(d); // Get next integer part
            putc('0' + integerPart); // Print next digit
            precision++;
            // Check if fractional part is zero
            if (std::fabs(d - integerPart) < 1e-100) // Tolerance for floating point comparison
                break;
        }
    }

    void vprintf(const char* fmt, va_list args)
    {
        int state = PRINTF_STATE_NORMAL;
        int length = PRINTF_LENGTH_DEFAULT;
        int radix = 10;
        bool sign = false;
        bool number = false;
        bool isFloat = false;

        while (*fmt)
        {
            switch (state)
            {
                case PRINTF_STATE_NORMAL:
                    switch (*fmt)
                    {
                        case '%':   state = PRINTF_STATE_LENGTH;
                                    break;
                        default:    putc(*fmt);
                                    break;
                    }
                    break;

                case PRINTF_STATE_LENGTH:
                    switch (*fmt)
                    {
                        case 'h':   length = PRINTF_LENGTH_SHORT;
                                    state = PRINTF_STATE_LENGTH_SHORT;
                                    break;
                        case 'l':   length = PRINTF_LENGTH_LONG;
                                    state = PRINTF_STATE_LENGTH_LONG;
                                    break;
                        default:    goto PRINTF_STATE_SPEC_;
                    }
                    break;

                case PRINTF_STATE_LENGTH_SHORT:
                    if (*fmt == 'h')
                    {
                        length = PRINTF_LENGTH_SHORT_SHORT;
                        state = PRINTF_STATE_SPEC;
                    }
                    else goto PRINTF_STATE_SPEC_;
                    break;

                case PRINTF_STATE_LENGTH_LONG:
                    if (*fmt == 'l')
                    {
                        length = PRINTF_LENGTH_LONG_LONG;
                        state = PRINTF_STATE_SPEC;
                    }
                    else goto PRINTF_STATE_SPEC_;
                    break;

                case PRINTF_STATE_SPEC:
                PRINTF_STATE_SPEC_:
                    switch (*fmt)
                    {
                        case 'c':   putc((char)va_arg(args, int));
                                    break;

                        case 's':   
                                    puts(va_arg(args, const char*));
                                    break;

                        case '%':   putc('%');
                                    break;

                        case 'd':
                        case 'i':   radix = 10; sign = true; number = true; isFloat = false;
                                    break;

                        case 'u':   radix = 10; sign = false; number = true; isFloat = false;
                                    break;

                        case 'X':
                        case 'x':
                        case 'p':   radix = 16; sign = false; number = true; isFloat = false;
                                    break;

                        case 'o':   radix = 8; sign = false; number = true; isFloat = false;
                                    break;

                        case 'f':   radix = 10; sign = true; number = false; isFloat = true;
                                    break;

                        // ignore invalid spec
                        default:    break;
                    }

                    if (number)
                    {
                        if (sign)
                        {
                            switch (length)
                            {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:     printf_signed(va_arg(args, int), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG:        printf_signed(va_arg(args, long), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG_LONG:   printf_signed(va_arg(args, long long), radix);
                                                            break;
                            }
                        }
                        else
                        {
                            switch (length)
                            {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:     printf_unsigned(va_arg(args, unsigned int), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG:        printf_unsigned(va_arg(args, unsigned  long), radix);
                                                            break;

                            case PRINTF_LENGTH_LONG_LONG:   printf_unsigned(va_arg(args, unsigned  long long), radix);
                                                            break;
                            }
                        }
                    } else if(isFloat){
                        switch(length){
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:
                            case PRINTF_LENGTH_LONG_LONG:
                                                            printf_double(va_arg(args, double));
                                                            break;

                            case PRINTF_LENGTH_LONG:        printf_double(va_arg(args, long double));
                                                            break;
                        }
                    }

                    // reset state
                    state = PRINTF_STATE_NORMAL;
                    length = PRINTF_LENGTH_DEFAULT;
                    radix = 10;
                    sign = false;
                    number = false;
                    isFloat = false;
                    break;
            }

            fmt++;
        }
    }

    void printf(const char* fmt, ...){
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