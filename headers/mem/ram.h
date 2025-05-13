#pragma once
#include <vector>

using namespace std;

namespace memory 
{
    extern std::vector<uint8_t> ram;

    void initialize();
    void read(uint16_t addr, uint8_t* buffer);
    void write(uint16_t addr, uint8_t data);
}
