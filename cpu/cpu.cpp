/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    cpu.cpp - handle CPU, opcodes etc
*/

#include "../headers/cpu/cpu.h"
#include "../headers/mem/ram.h"

using namespace cpu;

instruction instructions[256];

uint8_t A = 0; // Accumulator
uint8_t X = 0; // Index register X
uint8_t Y = 0; // Index register Y
uint8_t SP = 0xFD; // Stack pointer 
uint8_t PS = 0; // Processor status
uint16_t PC = 0; // Program Counter

// Handle different addressing modes
uint16_t cpu::addressing::immediate(CPU& c)
{
    return c.PC++;
}

uint16_t cpu::addressing::zeroPage(CPU& c)
{
    // Access an address in zero page
    return memory::read(c.PC++);
}

uint16_t cpu::addressing::zeroPageX(CPU& c)
{
    // Access an address + the offset stored in the X register in zero page
    return memory::read(memory::read(c.PC++) + c.X) & 0xFF;
}

uint16_t cpu::addressing::zeroPageY(CPU& c)
{
    // Access an address + the offset stored in the Y register in zero page
    uint8_t addr = memory::read(c.PC++);
    return memory::read(memory::read(c.PC++) + c.Y) & 0xFF;
}

uint16_t cpu::addressing::absolute(CPU& c)
{
    // Absolute addressing mode uses 16-bit addresses, hence why we need to read two bytes
    uint8_t low = memory::read(c.PC++);
    uint8_t high = memory::read(c.PC++);

    // Shift the high byte 8 bits to the left, making space for the 8 bits of the lower byte
    return (high << 8) | low;
}

uint16_t cpu::addressing::absoluteX(CPU& c)
{
    // Same as normal absolute, however the address gets offset by the value in the X register 
    uint8_t low = memory::read(c.PC++);
    uint8_t high = memory::read(c.PC++);

    return ((high << 8) | low) + c.X;
}

uint16_t cpu::addressing::absoluteY(CPU& c)
{
    // Same as normal absolute, however the address gets offset by the value in the Y register
    uint8_t low = memory::read(c.PC++);
    uint8_t high = memory::read(c.PC++);

    return ((high << 8) | low) + c.Y;
}

uint16_t cpu::addressing::indirect(CPU& c)
{
    // this is only used by JMP, pretty similar to absolute addressing mode but targetting a pointer instead
    uint8_t low = memory::read(c.PC++);
    uint8_t high = memory::read(c.PC++);

    uint8_t ptr = (high << 8 | low);

    // read the address in ptr
    uint8_t ptrLo = memory::read(ptr);
    uint8_t ptrHi = memory::read((ptr & 0xFF00) | ((ptr + 1) & 0x00FF)); // this replicates a bug with the 6502.

    return ((ptrHi << 8) | ptrLo);
}

uint16_t cpu::addressing::indirectX(CPU& c)
{
    // target is pointer in zero page, offset by X
    uint16_t base = (memory::read(c.PC++) + c.X) & 0xFF;
    uint8_t low = memory::read(base);
    uint8_t high = memory::read(base + 1) & 0xFF;

    return (high << 8) | low; 
}

uint16_t cpu::addressing::indirectY(CPU& c)
{
    // target is pointer in zero page, offset by Y
    uint16_t base = (memory::read(c.PC++) + c.Y) & 0xFF;
    uint8_t low = memory::read(base);
    uint8_t high = memory::read(base + 1) & 0xFF;

    return (high << 8) | low;
}

int16_t cpu::addressing::relative(CPU& c)
{
    // mainly used for branches
    // 8 bit SIGNED offset
    int8_t offset = memory::read(c.PC++);
    return static_cast<int16_t>(c.PC + offset);
}

void cpu::addressing::implied(CPU& c)
{
    // no operand (?)
}

uint8_t& cpu::addressing::accumulator(CPU& c)
{
    // The accumulator register will be the target operand
    return c.A;
}

void CPU::setFlag(flags flag, bool value)
{
    // activate/deactivate specific CPU flags using bit operations on the PS register
    if (value)
        PS |= flag; // sets the bit to 1 (ex: ps = 01110000, flag = 00000010 (zero flag), ps |= flag = 01110010)
    else
        PS &= ~flag; // invert the mask to set the bit to 0 (ex: ps = 01110010, flag = 00000010 now 11111101, ps &= flag = 01110000)
}

bool CPU::getFlag(flags flag) const
{
    // get current status of desired CPU flag
    return PS & flag;
}

void cpu::populate()
{
    // populate instructions
}

void cpu::initialize()
{
    cpu::populate();
}