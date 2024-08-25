#pragma once
#include <cstdint>

namespace framebuffer{
    void init();
    void setPixel(std::uint64_t x, std::uint64_t y, std::uint8_t r, std::uint8_t g, std::uint8_t b);
    void fill(std::uint8_t r, std::uint8_t g, std::uint8_t b);
    void putc(char c);
    void setFontSize(int size);
    void printInfo();
}