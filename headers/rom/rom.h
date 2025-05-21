/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

// basic (bad) ROM loader (?)
// uses the NES 2.0 format

#pragma once
#include <cstdint>
#include <vector>

namespace rom
{
    struct ROM
    {
        std::vector<uint8_t> header; // 16-byte header
        std::vector<uint8_t> trainer;
        std::vector<uint8_t> prg;
        std::vector<uint8_t> chr;
    };

    void testLoad();
}