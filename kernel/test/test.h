#pragma once
#include <utility>

namespace test{
    std::pair<const char*, bool> runDBG();
    std::pair<const char*, bool> runFB();
    std::pair<const char*, bool> runMMU();
    std::pair<const char*, bool> runACPI();
    std::pair<const char*, bool> runIRQ();
    std::pair<const char*, bool> runVFS();
    std::pair<const char*, bool> runPCI();
    std::pair<const char*, bool> runSYSCALL();
    std::pair<const char*, bool> runTSS();
    void runAll();
};