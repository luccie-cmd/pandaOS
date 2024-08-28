#pragma once
#include <cstdint>

namespace vfs{
    #define SECTOR_SIZE 512
    
    struct File{
        uint64_t handle;
        uint64_t size;
        uint64_t pos;
        bool isDir;
    };

    struct FileMeta{
        char buffer[SECTOR_SIZE];
        File publicData;
        bool opened;
        uint64_t currentCluster;
        uint64_t lastCluster;
        uint64_t currentSectorInCluster;
    };
};