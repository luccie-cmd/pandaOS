#pragma once
#include "mbr/mbr.h"
#include "part.h"   
#include <vector>

namespace vfs{
namespace disk{
namespace gpt{
    struct Gpt{
        mbr::Mbr prot_mbr;
        PartitionHeader header;
        std::vector<Parition*> partitions;
    };
};
};
};