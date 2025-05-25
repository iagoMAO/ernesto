/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    ram.cpp - handle memory (RAM) related actions, write/read etc
*/

#include "../headers/mem/ram.h";

namespace memory {
    std::vector<uint8_t> internal(0x0800); // 2kb
    std::vector<uint8_t> ppu(0x8);
    std::vector<uint8_t> apu(0x20);
    std::vector<uint8_t> prg;

    void initialize()
    {
        // init space for 2048 (2kb)
        // could improve mapping (?)
        // basically seperate memory blocks for easier 
        internal.resize(0x0800);
        ppu.resize(8);
        apu.resize(0x20);
        prg.resize(0x8000);

        std::fill(internal.begin(), internal.end(), 0xFF);
    }

    void write(uint16_t addr, uint8_t data)
    {
        // write to desired address
        if (addr < 0x2000)
            internal[addr % 0x0800] = data;
        else if (addr < 0x4000)
            ppu[(addr - 0x2000) % 8] = data;
        else if (addr < 0x4020)
            apu[addr - 0x4000] = data;
        else
            return;
    }

    uint8_t read(uint16_t addr)
    {
        if (addr < 0x2000)
            return internal[addr % 0x0800];
        else if (addr < 0x4000)
            return ppu[(addr - 0x2000) % 8];
        else if (addr < 0x4020)
            return apu[addr - 0x4000];
        else if (addr >= 0x8000)
        {
            size_t offset = addr - 0x8000;
            if (prg.size() == 0x4000)
                offset = offset % 0x4000;
            return prg[offset];
        }
        else
            return 0;
    }
}