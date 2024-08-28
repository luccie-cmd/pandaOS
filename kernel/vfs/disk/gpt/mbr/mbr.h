#pragma once

namespace vfs{
namespace disk{
namespace gpt{
namespace mbr{
    struct Mbr{
        char idc[510];
        char bootBytes[2];
    } __attribute__((packed));
};
};
};
};