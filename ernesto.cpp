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
        uint8_t opcode[3];
        opcode[0] = memory::read(c->PC);
        opcode[1] = memory::read(c->PC + 1);
        opcode[2] = memory::read(c->PC + 2);
        const cpu::CPU::instruction& instr = c->instructions[opcode[0]];

        printf("%04X  %02X %02X %02X  %s  A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", 
            c->PC,
            opcode[0],
            opcode[1],
            opcode[2],
            instr.name.c_str(),
            c->A,
            c->X,
            c->Y,
            c->PS,
            c->SP);

        if (opcode[0] == 0xFF && opcode[1] == 0xFF && opcode[2] == 0xFF)
            break;

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