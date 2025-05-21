/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"
#include "headers/cpu/cpu.h"
#include "headers/rom/rom.h"

int main()
{
    std::cout << "[ernesto] - welcome\n";

    memory::initialize();

    std::cout << "[ernesto] - RAM ok\n";

    // load a ROM into memory
    rom::testLoad();
    
    std::cout << "[ernesto] - nestest.nes loaded\n";

    cpu::CPU* c = cpu::initialize();
    c->PC = 0xC000;

    for (;;)
    {
        uint8_t opcode = memory::read(c->PC);
        const cpu::CPU::instruction& instr = c->instructions[opcode];

        printf("\n[ernesto] - [PC: %04X] opcode: %02X (%s) - A: %#04x | X: %#04x | Y: %#04x | SP: %#04x | P: %#08x", c->PC, opcode, instr.name.c_str(), c->A, c->X, c->Y, c->SP, c->PS);

        if (instr.impl)
        {
            instr.impl(*c, instr.mode);
            if (!instr.incrementPc)
                c->PC += instr.size;
        }
        else
        {
            printf("\n[ernesto] - unimplemented opcode: %x", (int)opcode);
            break;
        }
    };

    cin.get();
    return 0;
}