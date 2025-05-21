#pragma once
#include <vector>

using namespace std;

namespace memory 
{
    extern std::vector<uint8_t> internal; // 2kb
    extern std::vector<uint8_t> ppu;
    extern std::vector<uint8_t> apu;
    extern std::vector<uint8_t> prg;

    void initialize();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
}
