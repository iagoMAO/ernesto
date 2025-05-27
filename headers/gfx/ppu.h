/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#pragma once
#include "../mem/ram.h";
#include "../cpu/cpu.h";
#include <vector>

using namespace std;

namespace ppu
{
    extern std::vector<uint8_t> pattern_tables; // 8192 bytes
    extern std::vector<uint8_t> nametables; // 4096 bytes
    extern std::vector<uint8_t> mirror; // 3840 bytes
    extern std::vector<uint8_t> palette; // 32 bytes

    extern uint8_t* ppu_ctrl; // Writable
    extern uint8_t* ppu_mask; // Writable
    extern uint8_t* ppu_status; // Read only
    extern uint8_t* oam_addr; // Writable 
    extern uint8_t* oam_data; // Writable and readable
    extern uint16_t* ppu_scroll; // Writable 2x (16-bit)
    extern uint16_t* ppu_addr; // Writable 2x (16-bit)
    extern uint8_t* ppu_data; // Writable and readable
    extern uint8_t* oam_dma; // Writable

    extern uint32_t framebuffer[256 * 240];

    extern cpu::CPU* cpu;

    void tick();
    void initialize(cpu::CPU& c);

    void drawNametable(uint32_t* fb, const uint8_t* pattern_tables, const uint8_t* nametables, const uint8_t* palette);
    void write(uint16_t addr, uint8_t value);
    uint8_t read(uint16_t addr);

    extern bool frameReady;

    enum ppu_ctrl_flags
    {
        V = 0x80, // NMI enable
        P = 0x40, // PPU master/slave
        H = 0x20, // sprite height
        B = 0x10, // background tile select
        S = 0x08, // sprite tile select
        I = 0x04, // increment mode
        NN = 0x03, // nametable select / X and Y scroll bit 
    };

    enum ppu_mask_flags
    {
        BGR = 0xE0, // Color emphasis (BGR)
        s = 0x10, // Sprite enable
        b = 0x08, // Background enable
        M = 0x04, // Sprite left column enable
        m = 0x02, // Background left column enable
        G = 0x01, // Greyscale
    };

    enum ppu_status_flags
    {
        s_V = 0x80, // V-Blank
        s_S = 0x40, // Sprite 0 hit
        s_O = 0x20, // Sprite overflow
    };
}