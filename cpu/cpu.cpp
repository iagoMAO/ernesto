/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    cpu.cpp - handle CPU, opcodes etc
*/

#include "../headers/cpu/cpu.h"
#include "../headers/mem/ram.h"

namespace cpu
{
    instruction instructions[256];

    namespace addressing
    {
        // Handle different addressing modes
        uint8_t immediate(cpu& cpu)
        {
            return memory::read(cpu.PC++);
        }

        uint8_t zeroPage(cpu& cpu)
        {
            // Access an address in zero page
            uint8_t addr = memory::read(cpu.PC++);
            return memory::read(addr);
        }

        uint8_t zeroPageX(cpu& cpu)
        {
            // Access an address + the offset stored in the X register in zero page
            uint8_t addr = memory::read(cpu.PC++);
            return memory::read(addr + cpu.X);
        }

        uint8_t zeroPageY(cpu& cpu)
        {
            // Access an address + the offset stored in the Y register in zero page
            uint8_t addr = memory::read(cpu.PC++);
            return memory::read(addr + cpu.Y);
        }
    }

    struct cpu
    {
        uint8_t A = 0; // Accumulator
        uint8_t X = 0; // Index register X
        uint8_t Y = 0; // Index register Y
        uint8_t SP = 0xFD; // Stack pointer 
        uint8_t PS = 0; // Processor status
        uint16_t PC = 0; // Program Counter
        
        enum flags
        {
            N = 0x80, // negative flag
            V = 0x40, // overlfow flag
            U = 0x20, // unused
            B = 0x10, // break
            D = 0x08, // decimal mode
            I = 0x04, // interrupt disable
            Z = 0x02, // zero
            C = 0x01 // carry
        };

        inline void setFlag(flags flag, bool value)
        {
            // activate/deactivate specific CPU flags using bit operations on the PS register
            if (value)
                PS |= flag; // sets the bit to 1 (ex: ps = 01110000, flag = 00000010 (zero flag), ps |= flag = 01110010)
            else
                PS &= ~flag; // invert the mask to set the bit to 0 (ex: ps = 01110010, flag = 00000010 now 11111101, ps &= flag = 01110000)
        }

        inline bool getFlag(flags flag) const
        {
            // get current status of desired CPU flag
            return PS & flag;
        }

        void initialize()
        {
            populate();
        }

        void populate()
        {
            // populate instructions
        }
    };
}