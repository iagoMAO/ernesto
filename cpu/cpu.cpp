/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    cpu.cpp - handle CPU, opcodes etc
*/

#include "../headers/cpu/cpu.h"
#include "../headers/mem/ram.h"

using namespace cpu;

cpu::CPU::instruction instructions[256];

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

uint16_t cpu::addressing::resolve(CPU& c, CPU::addressingMode mode)
{
    switch (mode)
    {
    case CPU::Absolute:
        return cpu::addressing::absolute(c);
    case CPU::AbsoluteX:
        return cpu::addressing::absoluteX(c);
    case CPU::AbsoluteY:
        return cpu::addressing::absoluteY(c);
    case CPU::Accumulator:
        return cpu::addressing::accumulator(c);
    case CPU::IdxIndirect:
        return cpu::addressing::indirectX(c);
    case CPU::IndirectIdx:
        return cpu::addressing::indirectY(c);
    case CPU::Indirect:
        return cpu::addressing::indirect(c);
    case CPU::Immediate:
        return cpu::addressing::immediate(c);
    case CPU::Relative:
        return cpu::addressing::relative(c);
    case CPU::ZeroPage:
        return cpu::addressing::zeroPage(c);
    case CPU::ZeroPageX:
        return cpu::addressing::zeroPageX(c);
    case CPU::ZeroPageY:
        return cpu::addressing::zeroPageY(c);
    }
}

// LDA - loads the content of operand into the accumulator register
void cpu::opcodes::LDA(CPU& c, CPU::addressingMode mode)
{
    // bear in mind: all addressing modes return unsigned integers except for relative mode
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    c.A = operand;
    c.setFlag(CPU::Z, (operand == 0));
    c.setFlag(CPU::N, (operand && 0x80)); // check last bit for sign
}

// STA - stores the content of the accumulator register into memory
void cpu::opcodes::STA(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    memory::write(address, c.A);
}

// LDX - loads the content of operand into the X register
void cpu::opcodes::LDX(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    c.X = operand;
    c.setFlag(CPU::Z, (operand == 0));
    c.setFlag(CPU::N, (operand && 0x80)); // check last bit for sign
}

// STX - stores the content of the X register into memory
void cpu::opcodes::STX(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    memory::write(address, c.X);
}

// LDY - loads the content of operand into the Y register
void cpu::opcodes::LDY(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    c.Y = operand;
    c.setFlag(CPU::Z, (operand == 0));
    c.setFlag(CPU::N, (operand && 0x80)); // check last bit for sign
}

// STY - stores the content of the Y register into memory
void cpu::opcodes::STY(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    memory::write(address, c.Y);
}

// TAX - transfers X to A
void cpu::opcodes::TAX(CPU& c, CPU::addressingMode mode)
{
    // The addressing mode here is implied, hence no need for resolving
    c.X = c.A;
    c.setFlag(CPU::Z, (c.X == 0));
    c.setFlag(CPU::N, (c.X && 0x80)); // check last bit for sign
}

// TXA - transfers A to X
void cpu::opcodes::TXA(CPU& c, CPU::addressingMode mode)
{
    // The addressing mode here is implied, hence no need for resolving
    c.A = c.X;
    c.setFlag(CPU::Z, (c.A == 0));
    c.setFlag(CPU::N, (c.A && 0x80)); // check last bit for sign
}

// TAY - transfers Y to A
void cpu::opcodes::TAY(CPU& c, CPU::addressingMode mode)
{
    // The addressing mode here is implied, hence no need for resolving
    c.Y = c.A;
    c.setFlag(CPU::Z, (c.Y == 0));
    c.setFlag(CPU::N, (c.Y && 0x80)); // check last bit for sign
}

// TYA - transfers A to Y
void cpu::opcodes::TYA(CPU& c, CPU::addressingMode mode)
{
    // The addressing mode here is implied, hence no need for resolving
    c.A = c.Y;
    c.setFlag(CPU::Z, (c.A == 0));
    c.setFlag(CPU::N, (c.A && 0x80)); // check last bit for sign
}

// ADC - Add with Carry
void cpu::opcodes::ADC(CPU& c, CPU::addressingMode mode)
{
    // A = A + memory + C
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    uint8_t result = (c.A + memory::read(operand) + (c.C ? 1 : 0));
    c.A = result;

    c.setFlag(CPU::C, (result > 0xFF));
    c.setFlag(CPU::Z, (result == 0));
    c.setFlag(CPU::V, ((result ^ c.A) & (result ^ operand)) & 0x80); // check for signed overflow
    c.setFlag(CPU::N, (result && 0x80)); // if negative
}

// SBC - Subtract with Carry
void cpu::opcodes::SBC(CPU& c, CPU::addressingMode mode)
{
    // A = A - memory - ~C
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    uint8_t result = (c.A - memory::read(operand) - ~(c.C ? 1 : 0));
    c.A = result;

    c.setFlag(CPU::C, !(result < 0x00));
    c.setFlag(CPU::Z, (result == 0));
    c.setFlag(CPU::V, ((result ^ c.A) & (result ^ ~operand)) & 0x80); // check for signed overflow
    c.setFlag(CPU::N, (result && 0x80)); // if negative
}

// INC - Increment memory
void cpu::opcodes::INC(CPU& c, CPU::addressingMode mode)
{
    // memory = memory + 1
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    
    // write the original value first
    memory::write(address, operand);

    // now we increment
    memory::write(address, operand + 1);

    c.setFlag(CPU::Z, (operand + 1) == 0);
    c.setFlag(CPU::N, (operand + 1) && 0x80);
}

// INX - Increment X
void cpu::opcodes::INX(CPU& c, CPU::addressingMode mode)
{
    // x = x + 1
    c.X += 1;
    c.setFlag(CPU::Z, c.X == 0);
    c.setFlag(CPU::N, c.X && 0x80);
}

// INY - Increment Y
void cpu::opcodes::INY(CPU& c, CPU::addressingMode mode)
{
    // y = y + 1
    uint8_t result = c.Y += 1;
    c.Y = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// DEC - Decrement memory
void cpu::opcodes::DEC(CPU& c, CPU::addressingMode mode)
{
    // memory = memory - 1
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    // write the original value first
    memory::write(address, operand);

    // now we increment
    memory::write(address, operand - 1);

    c.setFlag(CPU::Z, (operand - 1) == 0);
    c.setFlag(CPU::N, (operand - 1) && 0x80);
}

// DEX - Decrement X
void cpu::opcodes::DEX(CPU& c, CPU::addressingMode mode)
{
    // x = x + 1
    uint8_t result = c.X -= 1;
    c.X = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// DEY - Decrement Y
void cpu::opcodes::DEY(CPU& c, CPU::addressingMode mode)
{
    // y = y + 1
    uint8_t result = c.Y -= 1;
    c.Y = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
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

CPU* cpu::initialize()
{
    CPU* c = new CPU();
    cpu::populate();

    return c;
}