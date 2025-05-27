/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include "../headers/gfx/ppu.h";
#include <cstdint>
#include <ctime>

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
    bool addrLatch = false;

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

    uint16_t vramAddress = 0;
    uint16_t tempVramAddress = 0;

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

        srand(time(nullptr)); // Seed random number generator

        for (int i = 0; i < 32; i++) {
            ppu::palette[i] = rand() % 64;  // Random NES palette index (0-63)
        }

        for (int i = 0; i < 960; ++i)
        {
            ppu::nametables[i] = i % 256;
        }
    }

    void tick()
    {
        frameReady = false;
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

                drawNametable(framebuffer, ppu::pattern_tables.data(), ppu::nametables.data(), ppu::palette.data());
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

    uint8_t read(uint16_t addr)
    {
        uint16_t reg = (addr - 0x2000) % 8;
        uint8_t status = *ppu_status;

        switch (reg)
        {
        case 2:
            *ppu_status &= ~0x80;
            addrLatch = false;
            return status;
        case 4:
            return *oam_data;
        case 7:
            // TODO: delayed read buffer behavior
            if (*ppu_ctrl & 0x04)
                vramAddress += 32;
            else
                vramAddress += 1;

            vramAddress &= 0x3FFF;

            return *ppu_data;
        default:
            return 0x00;
        }
    }

    void write(uint16_t addr, uint8_t value)
    {
        uint16_t reg = (addr - 0x2000) % 8;
        switch (reg)
        {
        case 0: // PPUCTRL
            *ppu_ctrl = value;
            break;
        case 1: // PPUMASK
            *ppu_mask = value;
            break;
        case 2: // PPUSTATUS
            return;
            break;
        case 3:
            *oam_addr = value;
            break;
        case 4:
            *oam_data = value;
            break;
        case 5:
            // TODO
            break;
            // Fixed PPUADDR write logic (case 6)
        case 6:
            printf("PPUADDR write BEFORE: value = 0x%02X, addrLatch = %d, tempVramAddress = 0x%04X, vramAddress = 0x%04X\n",
                value, addrLatch, tempVramAddress, vramAddress);

            if (!addrLatch)
            {
                // First write: high byte (clear low byte, set high byte)
                tempVramAddress = ((value & 0x3F) << 8);
                addrLatch = true;
            }
            else
            {
                // Second write: low byte
                tempVramAddress = (tempVramAddress & 0xFF00) | value;
                vramAddress = tempVramAddress;
                addrLatch = false;
            }

            printf("PPUADDR write AFTER: tempVramAddress = 0x%04X, vramAddress = 0x%04X, addrLatch = %d\n",
                tempVramAddress, vramAddress, addrLatch);
            break;

            // Fixed PPUDATA write logic (case 7)
        case 7:
            printf("PPUDATA write: addr=0x%04X, value=0x%02X", vramAddress, value);

            // Debug the conditional logic
            printf(" [DEBUG: addr < 0x2000? %s]", (vramAddress < 0x2000) ? "YES" : "NO");
            printf(" [DEBUG: addr >= 0x2000 && < 0x3000? %s]", (vramAddress >= 0x2000 && vramAddress < 0x3000) ? "YES" : "NO");
            printf(" [DEBUG: addr >= 0x3000 && < 0x3F00? %s]", (vramAddress >= 0x3000 && vramAddress < 0x3F00) ? "YES" : "NO");

            // Handle different memory regions
            if (vramAddress < 0x2000)
            {
                // Pattern tables - for CHR-ROM games like Donkey Kong, these writes should be ignored
                printf(" -> Ignoring pattern table write (CHR-ROM)\n");
                // pattern_tables[vramAddress] = value; // Don't write to CHR-ROM
            }
            else if (vramAddress >= 0x2000 && vramAddress < 0x3000)
            {
                // Nametables
                uint16_t ntAddr = vramAddress - 0x2000;
                // Apply proper mirroring based on cartridge type
                // For now, assuming vertical mirroring (common for Donkey Kong)
                if (ntAddr >= 0x800) {
                    ntAddr = (ntAddr - 0x800) % 0x800; // Mirror 0x2800-0x2FFF to 0x2000-0x27FF
                }
                if (ntAddr >= 0x400) {
                    ntAddr = ntAddr % 0x800; // Keep within bounds
                }
                nametables[ntAddr] = value;
                printf(" -> Writing to nametable: mapped=0x%04X\n", ntAddr);
            }
            else if (vramAddress >= 0x3000 && vramAddress < 0x3F00)
            {
                // Mirror of nametables (0x3000-0x3EFF mirrors 0x2000-0x2EFF)
                uint16_t mirrorAddr = vramAddress - 0x1000; // 0x3000 -> 0x2000
                // Recursively call the nametable write logic
                uint16_t ntAddr = mirrorAddr - 0x2000;
                if (ntAddr >= 0x800) {
                    ntAddr = (ntAddr - 0x800) % 0x800;
                }
                if (ntAddr >= 0x400) {
                    ntAddr = ntAddr % 0x800;
                }
                nametables[ntAddr] = value;
                printf(" -> Writing to nametable mirror: mapped=0x%04X\n", ntAddr);
            }
            else if (vramAddress >= 0x3F00 && vramAddress < 0x4000)
            {
                // Palette RAM - FIXED LOGIC
                uint16_t paletteAddr = (vramAddress - 0x3F00) & 0x1F; // Get offset from 0x3F00 and wrap to 32 bytes

                // Handle palette mirroring correctly
                // Background palette mirroring: 0x3F10, 0x3F14, 0x3F18, 0x3F1C mirror to 0x3F00, 0x3F04, 0x3F08, 0x3F0C
                if (paletteAddr == 0x10 || paletteAddr == 0x14 || paletteAddr == 0x18 || paletteAddr == 0x1C) {
                    paletteAddr = paletteAddr & 0x0F; // Mirror to background palette
                }

                palette[paletteAddr] = value & 0x3F; // Store with 6-bit mask
                printf(" -> Writing to palette: addr=0x%04X, mapped=0x%02X, value=0x%02X\n", vramAddress, paletteAddr, value & 0x3F);
            }
            else
            {
                printf(" -> Unknown address range!\n");
            }

            // Increment VRAM address
            if (*ppu_ctrl & 0x04) {
                vramAddress += 32;
            }
            else {
                vramAddress += 1;
            }

            vramAddress &= 0x3FFF; // Wrap to 14 bits
            break;
        }
    }

    void drawNametable(uint32_t* framebuffer, const uint8_t* patternTables, const uint8_t* nametables, const uint8_t* paletteRAM) {
        // Determine which pattern table to use (PPUCTRL bit 4)
        const bool usePatternTable1 = (*ppu_ctrl & ppu_ctrl_flags::B) != 0;
        const uint8_t* activePatternTable = usePatternTable1 ? patternTables + 0x1000 : patternTables;

        for (uint16_t row = 0; row < 30; ++row) {
            for (uint16_t col = 0; col < 32; ++col) {
                // Get the tile index (0-255) from the nametable
                const uint16_t tileIndex = nametables[row * 32 + col];
                const uint8_t* tile = &activePatternTable[tileIndex * 16]; // Each tile is 16 bytes

                // Get the correct palette from the attribute table
                const uint16_t attrTableIndex = 0x3C0 + (row / 4) * 8 + (col / 4);
                const uint8_t attrByte = nametables[attrTableIndex];

                // Determine which 2 bits to use for this 2x2 tile block
                const uint8_t quadrantRow = (row % 4) / 2;
                const uint8_t quadrantCol = (col % 4) / 2;
                const uint8_t shift = (quadrantRow * 2 + quadrantCol) * 2;
                const uint8_t paletteIndex = (attrByte >> shift) & 0x03;

                // Render the 8x8 tile
                for (uint8_t y = 0; y < 8; ++y) {
                    const uint8_t plane0 = tile[y];       // Low bitplane
                    const uint8_t plane1 = tile[y + 8];   // High bitplane

                    for (uint8_t x = 0; x < 8; ++x) {
                        const uint8_t bit0 = (plane0 >> (7 - x)) & 1;
                        const uint8_t bit1 = (plane1 >> (7 - x)) & 1;
                        uint8_t colorIndex = (bit1 << 1) | bit0;

                        uint8_t finalColorIndex;

                        // FIXED: Proper palette indexing
                        if (colorIndex == 0) {
                            // Transparent - use universal background color (palette[0])
                            finalColorIndex = paletteRAM[0];
                        }
                        else {
                            // Use background palette: palette entries 1-3 for palette 0, 5-7 for palette 1, etc.
                            finalColorIndex = paletteRAM[1 + (paletteIndex * 4) + (colorIndex - 1)];
                        }

                        // Clamp to NES palette range (0-63)
                        finalColorIndex &= 0x3F;

                        // Calculate screen position
                        const uint16_t screenX = col * 8 + x;
                        const uint16_t screenY = row * 8 + y;

                        // Only draw if within bounds
                        if (screenX < 256 && screenY < 240) {
                            framebuffer[screenY * 256 + screenX] = nes_palette[finalColorIndex];
                        }
                    }
                }
            }
        }
    }
}