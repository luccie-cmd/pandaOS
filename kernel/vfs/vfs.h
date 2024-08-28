#pragma once
#include "file.h"

namespace vfs{
    void init();
    File* open(const char* path);
    void close(File* f);
};