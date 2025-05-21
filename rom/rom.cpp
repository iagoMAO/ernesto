/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

// basic (bad) ROM loader (?)
// uses the NES 2.0 format

#include <iostream>
#include <fstream>
#include "../headers/rom/rom.h"
#include "../headers/mem/ram.h"

namespace rom
{
    void testLoad()
    {
        ROM rom;
        std::ifstream file("I:\\Projects\\hobbies\\ernesto\\rom\\nestest2.nes", std::ios::binary);

        rom.header.resize(16);
        file.read(reinterpret_cast<char*>(rom.header.data()), 16); // load first 16 bytes of rom into the header

        // implement checking for valid NES rom l8r (check for NES magic)

        // prg size is stored in the 4th byte of the header
        int prgSize = rom.header[4] * 16 * 1024;
        int chrSize = rom.header[5] * 8 * 1024;
        
        // trainer section precedes PRG if 2nd bit of the 6th byte of the header is set
        bool trainer = rom.header[6] & 0x04;

        if (trainer)
        {
            rom.trainer.resize(512); // trainer sec always 512 bytes
            file.read(reinterpret_cast<char*>(rom.trainer.data()), 512);
        }

        rom.prg.resize(prgSize);
        file.read(reinterpret_cast<char*>(rom.prg.data()), prgSize);

        if (chrSize > 0)
        {
            rom.chr.resize(chrSize);
            file.read(reinterpret_cast<char*>(rom.chr.data()), chrSize);
        }

        // move PRG ROM into memory
        memory::prg = rom.prg;
        file.close();
    }
}