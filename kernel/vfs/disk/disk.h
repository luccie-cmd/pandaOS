#pragma once
#include "gpt/gpt.h"

namespace vfs{
namespace disk{
    struct Disk{
        gpt::Gpt gpt;
    };
    void init();
};
};