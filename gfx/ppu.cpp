/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include "../headers/gfx/ppu.h";
#include <cstdint>

namespace ppu
{
    int ppu_cycle = 0;
    int ppu_scanline = 0;

    uint8_t* ppu_ctrl; // Writable
    uint8_t* ppu_mask; // Writable
    uint8_t* ppu_status; // Read only
    uint8_t* oam_addr; // Writable 
    uint8_t* oam_data; // Writable and readable
    uint16_t* ppu_scroll; // Writable 2x (16-bit)
    uint16_t* ppu_addr; // Writable 2x (16-bit)
    uint8_t* ppu_data; // Writable and readable
    uint8_t* oam_dma; // Writable

    std::vector<uint8_t> pattern_tables(0x2000); // 8192 bytes
    std::vector<uint8_t> nametables(0x1000); // 4096 bytes
    std::vector<uint8_t> mirror(0x0F00); // 3840 bytes
    std::vector<uint8_t> palette(0x20); // 32 bytes

    cpu::CPU* cpu = nullptr;
    uint32_t framebuffer[256 * 240] = { 0 };

    bool frameReady = false;

    const uint32_t nes_palette[64] = {
        0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
        0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
        0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
        0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
        0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
        0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
        0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
        0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
    };

    void initialize(cpu::CPU& c)
    {
        // todo: better checks later for memory initialization, cpu status, etc
        // PPU registers must point to their respective addresses in memory map
        ppu_ctrl = &memory::ppu[0x00];
        ppu_mask = &memory::ppu[0x01];
        ppu_status = &memory::ppu[0x02];
        oam_addr = &memory::ppu[0x03];
        oam_data = &memory::ppu[0x04];
        ppu_scroll = reinterpret_cast<uint16_t*>(&memory::ppu[0x05]);
        ppu_addr = reinterpret_cast<uint16_t*>(&memory::ppu[0x06]);
        ppu_data = &memory::ppu[0x07];
        oam_dma = &memory::apu[0x14];

        cpu = &c;

        for (int i = 0; i < 960; ++i)
        {
            ppu::nametables[i] = i % 256;
        }

        for (int i = 0; i < 32; ++i)
        {
            ppu::palette[i] = i % 64;
        }
    }

    void tick()
    {
        ppu_cycle++;

        if (ppu_cycle >= 341) // this line is finished
        {
            ppu_cycle = 0;
            ppu_scanline++;

            if (ppu_scanline == 241)
            {
                // vblank
                *ppu_status |= ppu_status_flags::s_V;
                cpu::NMI(*cpu);

                for (int y = 0; y < 30; ++y)
                {
                    for (int x = 0; x < 32; ++x)
                    {
                        int tileIndex = x + y * 32;
                        testTile(ppu::nametables[tileIndex], x * 8, y * 8);
                    }
                }
            }
            else if (ppu_scanline >= 261)
            {
                // end of frame
                *ppu_status &= ~ppu_status_flags::s_V;
                ppu_scanline = 0;
                frameReady = true;
            }
        }
    }

    void testTile(int tileIndex, int screenX, int screenY)
    {
        int base = tileIndex * 16; // each tile is 16 bytes

        for (int y = 0; y < 8; ++y)
        {
            uint8_t plane0 = ppu::pattern_tables[base + y]; // bitplane 0
            uint8_t plane1 = ppu::pattern_tables[base + y + 8]; // bitplane 1

            for (int x = 0; x < 8; ++x)
            {
                uint8_t bit0 = (plane0 >> (7 - x)) & 1; // extract bit from left to right
                uint8_t bit1 = (plane1 >> (7 - x)) & 1;

                uint8_t colorIndex = (bit1 << 1) | bit0; // combine bitplanes to get pixel value (0-3)

                uint8_t paletteIndex = ppu::nametables[tileIndex];

                uint32_t color = (colorIndex == 0) ? 0xFF000000 : 0xFFFFFFFF; // RGBA color

                int index = (y + screenY) * 256 + (x + screenX); // calculate framebuffer position
                ppu::framebuffer[index] = color;
            }
        }
    }
}