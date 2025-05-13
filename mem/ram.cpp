/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    ram.cpp - handle memory (RAM) related actions, write/read etc
*/
#include "../headers/mem/ram.h";

namespace memory {
    std::vector<uint8_t> ram(2048);

    void initialize()
    {
        // init space for 2048 (2kb)
        // could improve mapping (?)
        // basically seperate memory blocks for easier 
        ram.resize(2048);
    }

    void write(uint16_t addr, uint8_t data)
    {
        // write to desired address
        if (addr >= ram.size()) return;
        ram[addr] = data;
    }

    uint8_t read(uint16_t addr)
    {
        if (addr >= ram.size()) return;
        return ram[addr];
    }
}