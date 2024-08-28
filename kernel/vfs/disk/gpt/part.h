#pragma once

namespace vfs{
namespace disk{
namespace gpt{
    struct PartitionHeader{
        char Signature[8];
        uint32_t Revision;
        uint32_t HeaderSize;
        uint32_t CRC32;
        uint32_t Reserved;
        uint64_t CurrentLBA;
        uint64_t BackupLBA;
        uint64_t FirstUseLBA;
        char GUID[16];
        uint64_t StartPartitions;
        uint32_t PartitionCount;
        uint32_t PartitionSize;
        uint32_t CRC32Part;
        char Unused[420];
    } __attribute__((packed));
    struct Parition{
        char TypeGUID[16];
        char PartitionGUID[16];
        uint64_t FirstLBA;
        uint64_t LastLBA;
        uint64_t Attributes;
        wchar_t Name[36];
    } __attribute__((packed));
};
};
};