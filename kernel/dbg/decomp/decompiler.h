#pragma once

extern "C"
{
#include <xed/xed-interface.h>
}

#include <cstddef>
#include <cstdint>

void DecompilerDecompileAtRIPRange(xed_decoded_inst_t* significant_instruction, const uint8_t* rip, std::size_t range_before, std::size_t range_after);