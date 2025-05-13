#pragma once
#include <vector>

using namespace std;

namespace memory 
{
    extern std::vector<uint8_t> ram;

    void initialize();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
}
