/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)

    cpu.cpp - handle CPU, false, opcodes etc
*/

#include "../headers/cpu/cpu.h"
#include "../headers/mem/ram.h"

using namespace cpu;

cpu::CPU::instruction instructions[256];

void cpu::NMI(CPU& c)
{
    c.pushByte((c.PC >> 8) & 0xFF); // high byte
    c.pushByte(c.PC & 0xFF); // low byte

    uint8_t ps = c.PS | 0x20;
    ps &= ~0x10;
    
    c.pushByte(ps);

    uint8_t nmi_lo = memory::read(0xFFFA);
    uint8_t nmi_hi = memory::read(0xFFFB);

    c.PC = (nmi_hi << 8) | nmi_lo;

    c.setFlag(CPU::I, true);
}

// Handle different addressing modes
uint16_t cpu::addressing::immediate(CPU& c)
{
    return c.PC + 1;
}

uint16_t cpu::addressing::zeroPage(CPU& c)
{
    // Access an address in zero page
    return memory::read(c.PC + 1);
}

uint16_t cpu::addressing::zeroPageX(CPU& c)
{
    // Access an address + the offset stored in the X register in zero page
    uint8_t addr = memory::read(c.PC + 1);
    return (addr + c.X) & 0xFF;
}

uint16_t cpu::addressing::zeroPageY(CPU& c)
{
    // Access an address + the offset stored in the Y register in zero page
    uint8_t addr = memory::read(c.PC + 1);
    return (addr + c.Y) & 0xFF;
}

uint16_t cpu::addressing::absolute(CPU& c)
{
    // Absolute addressing mode uses 16-bit addresses, hence why we need to read two bytes
    uint8_t low = memory::read(c.PC + 1);
    uint8_t high = memory::read(c.PC + 2);

    // Shift the high byte 8 bits to the left, making space for the 8 bits of the lower byte
    return (high << 8) | low;
}

uint16_t cpu::addressing::absoluteX(CPU& c)
{
    // Same as normal absolute, however the address gets offset by the value in the X register 
    uint8_t low = memory::read(c.PC + 1);
    uint8_t high = memory::read(c.PC + 2);

    return ((high << 8) | low) + c.X;
}

uint16_t cpu::addressing::absoluteY(CPU& c)
{
    // Same as normal absolute, however the address gets offset by the value in the Y register
    uint8_t low = memory::read(c.PC + 1);
    uint8_t high = memory::read(c.PC + 2);

    return ((high << 8) | low) + c.Y;
}

uint16_t cpu::addressing::indirect(CPU& c)
{
    // this is only used by JMP, pretty similar to absolute addressing mode but targetting a pointer instead
    uint8_t low = memory::read(c.PC + 1);
    uint8_t high = memory::read(c.PC + 2);

    uint16_t ptr = (high << 8 | low);

    // read the address in ptr
    uint8_t ptrL = memory::read(ptr);
    uint8_t ptrH = memory::read((ptr & 0xFF00) | ((ptr + 1) & 0x00FF));

    uint16_t result = (ptrH << 8 | ptrL);
    return result;
}

uint16_t cpu::addressing::indirectX(CPU& c)
{
    // target is pointer in zero page, offset by X
    uint16_t base = (memory::read(c.PC + 1) + c.X) & 0xFF;
    uint8_t low = memory::read(base);
    uint8_t high = memory::read(base + 1) & 0xFF;

    return (high << 8) | low; 
}

uint16_t cpu::addressing::indirectY(CPU& c)
{
    // target is pointer in zero page, offset by Y
    uint16_t base = (memory::read(c.PC + 1) + c.Y) & 0xFF;
    uint8_t low = memory::read(base);
    uint8_t high = memory::read(base + 1) & 0xFF;

    return (high << 8) | low;
}

int16_t cpu::addressing::relative(CPU& c)
{
    // mainly used for branches
    // 8 bit SIGNED offset
    return static_cast<int8_t>(memory::read(c.PC + 1));
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
    default:
        return 0; // ?
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

// ASL - Arithmetic shift left
void cpu::opcodes::ASL(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = mode == CPU::Accumulator ? c.A : memory::read(address);

    // we carry the last bit of the old value for 8-bit behavior
    c.setFlag(CPU::C, operand & 0x80);

    uint8_t result = operand << 1;

    if (mode != CPU::Accumulator)
        memory::write(address, result);
    else
        c.A = result;

    // set flags
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// LSR - Logical shift right
void cpu::opcodes::LSR(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = mode == CPU::Accumulator ? c.A : memory::read(address);

    // we carry the last bit of the old value for 8-bit behavior
    c.setFlag(CPU::C, operand & 0x01);

    uint8_t result = operand >> 1;

    if (mode != CPU::Accumulator)
        memory::write(address, result);
    else
        c.A = result;

    // set flags
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// ROL - Rotate Left
void cpu::opcodes::ROL(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = mode == CPU::Accumulator ? c.A : memory::read(address);

    c.setFlag(CPU::C, operand & 0x80);

    uint8_t result = (operand << 1) | (c.getFlag(CPU::C) ? 1 : 0);

    if (mode != CPU::Accumulator)
        memory::write(address, result);
    else
        c.A = result;

    c.setFlag(CPU::C, operand & 0x80);
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result & 0x80);
}

// ROR - Rotate Right
void cpu::opcodes::ROR(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = mode == CPU::Accumulator ? c.A : memory::read(address);

    c.setFlag(CPU::C, operand & 0x01);

    uint8_t result = (operand >> 1) | (c.getFlag(CPU::C) ? 0x80 : 0x00);

    if (mode != CPU::Accumulator)
        memory::write(address, result);
    else
        c.A = result;

    c.setFlag(CPU::C, operand & 0x01);
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result & 0x80);
}

// AND - Bitwise and
void cpu::opcodes::AND(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.A & operand;

    c.A = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// ORA - Bitwise or
void cpu::opcodes::ORA(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.A | operand;

    c.A = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// EOR - Bitwise exclusive or
void cpu::opcodes::EOR(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.A ^ operand;

    c.A = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result && 0x80);
}

// BIT - Bit test
void cpu::opcodes::BIT(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.A & operand;

    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::V, result && 0x70);
    c.setFlag(CPU::N, result && 0x80);
}

// CMP - Compare A
void cpu::opcodes::CMP(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.A - operand;

    c.setFlag(CPU::C, (c.A >= operand));
    c.setFlag(CPU::Z, (c.A == operand));
    c.setFlag(CPU::N, result && 0x80);
}

// CPX - Compare X
void cpu::opcodes::CPX(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.X - operand;

    c.setFlag(CPU::C, (c.X >= operand));
    c.setFlag(CPU::Z, (c.X == operand));
    c.setFlag(CPU::N, result && 0x80);
}

// CPY - Compare Y
void cpu::opcodes::CPY(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);
    uint8_t result = c.Y - operand;

    c.setFlag(CPU::C, (c.Y >= operand));
    c.setFlag(CPU::Z, (c.Y == operand));
    c.setFlag(CPU::N, result && 0x80);
}

// BCC - Branch if Carry Clear
void cpu::opcodes::BCC(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (!c.getFlag(CPU::C))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BCS - Branch if Carry Set
void cpu::opcodes::BCS(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (c.getFlag(CPU::C))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BCC - Branch if Equal
void cpu::opcodes::BEQ(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (c.getFlag(CPU::Z))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BNE - Branch if Not Equal
void cpu::opcodes::BNE(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (!c.getFlag(CPU::Z))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BPL - Branch if Plus
void cpu::opcodes::BPL(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (!c.getFlag(CPU::N))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BMI - Branch if Minus
void cpu::opcodes::BMI(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (c.getFlag(CPU::N))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BVC - Branch if Overflow Clear
void cpu::opcodes::BVC(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (!c.getFlag(CPU::V))
        c.PC = next + offset;
    else
        c.PC = next;
}

// BVS - Branch if Overflow Set
void cpu::opcodes::BVS(CPU& c, CPU::addressingMode mode)
{
    int8_t offset = static_cast<int8_t>(cpu::addressing::resolve(c, mode));

    uint16_t next = c.PC + 2;

    if (c.getFlag(CPU::V))
        c.PC = next + offset;
    else
        c.PC = next;
}

// JMP - Jump!
void cpu::opcodes::JMP(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint16_t operand = memory::read(address);

    c.PC = (mode == CPU::Absolute || mode == CPU::Indirect ? address : operand);
}

// JSR - Jump to subroutine
void cpu::opcodes::JSR(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint16_t operand = memory::read(address);
    uint16_t returnAddr = c.PC + 2;
    c.pushByte((returnAddr >> 8) & 0xFF);
    c.pushByte(returnAddr & 0xFF);

    c.PC = (mode == CPU::Absolute ? address : operand);
}

// RTS - Return from subroutine
void cpu::opcodes::RTS(CPU& c, CPU::addressingMode mode)
{
    uint16_t low = c.pullByte();
    uint16_t high = c.pullByte();
    c.PC = ((high << 8) | low) + 1;
}

// BRK - Break
void cpu::opcodes::BRK(CPU& c, CPU::addressingMode mode)
{
    // push PC + 2 to stack
    // push NV11DIZC flags to stack
    c.pushByte(c.PC + 1);
    c.pushByte(c.PS);
    c.PC = 0xFFFE;
    c.setFlag(CPU::I, true);
    c.setFlag(CPU::B, true);
}

// RTI - Return from Interrupt
void cpu::opcodes::RTI(CPU& c, CPU::addressingMode mode)
{
    // TO-DO: implement pushing to stack
    // currently also useless without stack functionality
    uint8_t ps = c.pullByte();
    uint16_t low = c.pullByte();
    uint16_t high = c.pullByte();
    c.PC = ((high << 8) | low);
    c.PS = ps;
}

// PHA - Push A
void cpu::opcodes::PHA(CPU& c, CPU::addressingMode mode)
{
    c.pushByte(c.A);
}

// PLA - Pull A
void cpu::opcodes::PLA(CPU& c, CPU::addressingMode mode)
{
    uint16_t result = c.pullByte();
    c.A = result;
    c.setFlag(CPU::Z, result == 0);
    c.setFlag(CPU::N, result & 0x80);
}

// PHP - Push processor status
void cpu::opcodes::PHP(CPU& c, CPU::addressingMode mode)
{
    c.pushByte(c.PS);
}

// PLP - Pull processor status
void cpu::opcodes::PLP(CPU& c, CPU::addressingMode mode)
{
    uint8_t ps = c.pullByte();
    c.PS = ps;
}

// TXS - Transfer X to Stack Pointer
void cpu::opcodes::TXS(CPU& c, CPU::addressingMode mode)
{
    c.SP = c.X;
}

// TSX - Transfer Stack Pointer to X
void cpu::opcodes::TSX(CPU& c, CPU::addressingMode mode)
{
    c.X = c.SP;
    c.setFlag(CPU::Z, c.X == 0);
    c.setFlag(CPU::N, c.X & 0x80);
}

// CLC - Clear Carry
void cpu::opcodes::CLC(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::C, 0);
}

// CLD - Clear Decimal
void cpu::opcodes::CLD(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::D, 0);
}

// CLI - Clear Interrupt
void cpu::opcodes::CLI(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::I, 0);
}

// CLV - Clear Overflow
void cpu::opcodes::CLV(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::V, 0);
}

// SEC - Set Carry
void cpu::opcodes::SEC(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::C, 1);
}

// SED - Set Decimal
void cpu::opcodes::SED(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::D, 1);
}

// SEI - Set Interrupt
void cpu::opcodes::SEI(CPU& c, CPU::addressingMode mode)
{
    c.setFlag(CPU::I, 1);
}

// NOP - No operation
void cpu::opcodes::NOP(CPU& c, CPU::addressingMode mode)
{
    // Literally nothing
}

// LAX - Load A and X
void cpu::opcodes::LAX(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    c.A = operand;
    c.X = operand;
    c.setFlag(CPU::Z, (operand == 0));
    c.setFlag(CPU::N, (operand && 0x80)); // check last bit for sign

}

// SAX - Store A and X
void cpu::opcodes::SAX(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t value = c.A & c.X;
    memory::write(address, value);
}

// DCP - Decrements the operand and compares the result to the accumulator
void cpu::opcodes::DCP(CPU& c, CPU::addressingMode mode)
{
    uint16_t address = cpu::addressing::resolve(c, mode);
    uint8_t operand = memory::read(address);

    uint8_t value = (operand - 1);
    memory::write(address, value);

    uint8_t result = c.A - value;

    c.setFlag(CPU::C, (c.A >= value));
    c.setFlag(CPU::Z, (result == 0));
    c.setFlag(CPU::N, result & 0x80);
}

// ISC - INC + SBC
void cpu::opcodes::ISC(CPU& c, CPU::addressingMode mode)
{
    INC(c, mode);
    SBC(c, mode);
}

// RLA - ROL + AND
void cpu::opcodes::RLA(CPU& c, CPU::addressingMode mode)
{
    ROL(c, mode);
    AND(c, mode);
}

// SLO - ASL + ORA
void cpu::opcodes::SLO(CPU& c, CPU::addressingMode mode)
{
    ASL(c, mode);
    ORA(c, mode);
}

// SRE - LSR + EOR
void cpu::opcodes::SRE(CPU& c, CPU::addressingMode mode)
{
    LSR(c, mode);
    EOR(c, mode);
}

// RRA - ROR + ADC
void cpu::opcodes::RRA(CPU& c, CPU::addressingMode mode)
{
    ROR(c, mode);
    ADC(c, mode);
}


uint8_t& cpu::addressing::accumulator(CPU& c)
{
    // The accumulator register will be the target operand
    return c.A;
}

void CPU::pushByte(uint16_t value)
{
    // Stack begins at 0x0100
    memory::write(0x0100 + SP, value);
    SP--;
}

uint16_t CPU::pullByte()
{
    SP++;
    return memory::read(0x0100 + SP);
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

void CPU::populate()
{
    // populate instructions
    cpu::CPU::instructions[0x69] = { "ADC", Immediate, 2, 2, false, opcodes::ADC };
    cpu::CPU::instructions[0x65] = { "ADC", ZeroPage, 2, 3, false, opcodes::ADC };
    cpu::CPU::instructions[0x75] = { "ADC", ZeroPageX, 2, 4, false, opcodes::ADC };
    cpu::CPU::instructions[0x6D] = { "ADC", Absolute, 3, 4, false, opcodes::ADC };
    cpu::CPU::instructions[0x7D] = { "ADC", AbsoluteX, 3, 4, false, opcodes::ADC };
    cpu::CPU::instructions[0x79] = { "ADC", AbsoluteY, 3, 4, false, opcodes::ADC };
    cpu::CPU::instructions[0x61] = { "ADC", IdxIndirect, 2, 6, false, opcodes::ADC };
    cpu::CPU::instructions[0x71] = { "ADC", IndirectIdx, 2, 5, false, opcodes::ADC };

    cpu::CPU::instructions[0x29] = { "AND", Immediate, 2, 2, false, opcodes::AND };
    cpu::CPU::instructions[0x25] = { "AND", ZeroPage, 2, 3, false, opcodes::AND };
    cpu::CPU::instructions[0x35] = { "AND", ZeroPageX, 2, 4, false, opcodes::AND };
    cpu::CPU::instructions[0x2D] = { "AND", Absolute, 3, 4, false, opcodes::AND };
    cpu::CPU::instructions[0x3D] = { "AND", AbsoluteX, 3, 4, false, opcodes::AND };
    cpu::CPU::instructions[0x39] = { "AND", AbsoluteY, 3, 4, false, opcodes::AND };
    cpu::CPU::instructions[0x21] = { "AND", IdxIndirect, 2, 6, false, opcodes::AND };
    cpu::CPU::instructions[0x31] = { "AND", IndirectIdx, 2, 5, false, opcodes::AND };

    cpu::CPU::instructions[0x0A] = { "ASL", Accumulator, 1, 2, false, opcodes::ASL };
    cpu::CPU::instructions[0x06] = { "ASL", ZeroPage, 2, 5, false, opcodes::ASL };
    cpu::CPU::instructions[0x16] = { "ASL", ZeroPageX, 2, 6, false, opcodes::ASL };
    cpu::CPU::instructions[0x0E] = { "ASL", Absolute, 3, 6, false, opcodes::ASL };
    cpu::CPU::instructions[0x1E] = { "ASL", AbsoluteX, 3, 7, false, opcodes::ASL };

    cpu::CPU::instructions[0x90] = { "BCC", Relative, 2, 1, true, opcodes::BCC };
    cpu::CPU::instructions[0xB0] = { "BCS", Relative, 2, 1, true, opcodes::BCS };
    cpu::CPU::instructions[0xF0] = { "BEQ", Relative, 2, 1, true, opcodes::BEQ };
    cpu::CPU::instructions[0x30] = { "BMI", Relative, 2, 1, true, opcodes::BMI };
    cpu::CPU::instructions[0xD0] = { "BNE", Relative, 2, 1, true, opcodes::BNE };
    cpu::CPU::instructions[0x10] = { "BPL", Relative, 2, 1, true, opcodes::BPL };
    cpu::CPU::instructions[0x50] = { "BVC", Relative, 2, 1, true, opcodes::BVC };
    cpu::CPU::instructions[0x70] = { "BVS", Relative, 2, 1, true, opcodes::BVS };

    cpu::CPU::instructions[0x00] = { "BRK", Implicit, 1, 7, false, opcodes::BRK };
    cpu::CPU::instructions[0x00] = { "BRK", Immediate, 1, 7, false, opcodes::BRK };

    cpu::CPU::instructions[0x18] = { "CLC", Implicit, 1, 2, false, opcodes::CLC };
    cpu::CPU::instructions[0xD8] = { "CLD", Implicit, 1, 2, false, opcodes::CLD };
    cpu::CPU::instructions[0x58] = { "CLI", Implicit, 1, 2, false, opcodes::CLI };
    cpu::CPU::instructions[0xB8] = { "CLV", Implicit, 1, 2, false, opcodes::CLV };

    cpu::CPU::instructions[0x24] = { "BIT", ZeroPage, 2, 3, false, opcodes::BIT };
    cpu::CPU::instructions[0x2C] = { "BIT", Absolute, 3, 4, false, opcodes::BIT };

    cpu::CPU::instructions[0xC9] = { "CMP", Immediate, 2, 2, false, opcodes::CMP };
    cpu::CPU::instructions[0xC5] = { "CMP", ZeroPage, 2, 3, false, opcodes::CMP };
    cpu::CPU::instructions[0xD5] = { "CMP", ZeroPageX, 2, 4, false, opcodes::CMP };
    cpu::CPU::instructions[0xCD] = { "CMP", Absolute, 3, 4, false, opcodes::CMP };
    cpu::CPU::instructions[0xDD] = { "CMP", AbsoluteX, 3, 4, false, opcodes::CMP };
    cpu::CPU::instructions[0xD9] = { "CMP", AbsoluteY, 3, 4, false, opcodes::CMP };
    cpu::CPU::instructions[0xC1] = { "CMP", IdxIndirect, 2, 6, false, opcodes::CMP };
    cpu::CPU::instructions[0xD1] = { "CMP", IndirectIdx, 2, 5, false, opcodes::CMP };

    cpu::CPU::instructions[0xE0] = { "CPX", Immediate, 2, 2, false, opcodes::CPX };
    cpu::CPU::instructions[0xE4] = { "CPX", ZeroPage, 2, 3, false, opcodes::CPX };
    cpu::CPU::instructions[0xEC] = { "CPX", Absolute, 3, 4, false, opcodes::CPX };

    cpu::CPU::instructions[0xC0] = { "CPY", Immediate, 2, 2, false, opcodes::CPY };
    cpu::CPU::instructions[0xC4] = { "CPY", ZeroPage, 2, 3, false, opcodes::CPY };
    cpu::CPU::instructions[0xCC] = { "CPY", Absolute, 3, 4, false, opcodes::CPY };

    cpu::CPU::instructions[0xC6] = { "DEC", ZeroPage, 2, 5, false, opcodes::DEC };
    cpu::CPU::instructions[0xD6] = { "DEC", ZeroPageX, 2, 6, false, opcodes::DEC };
    cpu::CPU::instructions[0xCE] = { "DEC", Absolute, 3, 6, false, opcodes::DEC };
    cpu::CPU::instructions[0xDE] = { "DEC", AbsoluteX, 3, 7, false, opcodes::DEC };

    cpu::CPU::instructions[0xCA] = { "DEX", Implicit, 1, 2, false, opcodes::DEX };
    cpu::CPU::instructions[0x88] = { "DEY", Implicit, 1, 2, false, opcodes::DEY };

    cpu::CPU::instructions[0xE6] = { "INC", ZeroPage, 2, 5, false, opcodes::INC };
    cpu::CPU::instructions[0xF6] = { "INC", ZeroPageX, 2, 6, false, opcodes::INC };
    cpu::CPU::instructions[0xEE] = { "INC", Absolute, 3, 6, false, opcodes::INC };
    cpu::CPU::instructions[0xFE] = { "INC", AbsoluteX, 3, 7, false, opcodes::INC };

    cpu::CPU::instructions[0xE8] = { "INX", Implicit, 1, 2, false, opcodes::INX };
    cpu::CPU::instructions[0xC8] = { "INY", Implicit, 1, 2, false, opcodes::INY };

    cpu::CPU::instructions[0x49] = { "EOR", Immediate, 2, 2, false, opcodes::EOR };
    cpu::CPU::instructions[0x45] = { "EOR", ZeroPage, 2, 3, false, opcodes::EOR };
    cpu::CPU::instructions[0x55] = { "EOR", ZeroPageX, 2, 4, false, opcodes::EOR };
    cpu::CPU::instructions[0x4D] = { "EOR", Absolute, 3, 4, false, opcodes::EOR };
    cpu::CPU::instructions[0x5D] = { "EOR", AbsoluteX, 3, 4, false, opcodes::EOR };
    cpu::CPU::instructions[0x59] = { "EOR", AbsoluteY, 3, 4, false, opcodes::EOR };
    cpu::CPU::instructions[0x41] = { "EOR", IdxIndirect, 2, 6, false, opcodes::EOR };
    cpu::CPU::instructions[0x51] = { "EOR", IndirectIdx, 2, 5, false, opcodes::EOR };

    cpu::CPU::instructions[0x4C] = { "JMP", Absolute, 3, 3, true, opcodes::JMP };
    cpu::CPU::instructions[0x6C] = { "JMP", Indirect, 3, 5, true, opcodes::JMP };
    cpu::CPU::instructions[0x20] = { "JSR", Absolute, 3, 6, true, opcodes::JSR };

    cpu::CPU::instructions[0xA9] = { "LDA", Immediate, 2, 2, false, opcodes::LDA };
    cpu::CPU::instructions[0xA5] = { "LDA", ZeroPage, 2, 3, false, opcodes::LDA };
    cpu::CPU::instructions[0xB5] = { "LDA", ZeroPageX, 2, 4, false, opcodes::LDA };
    cpu::CPU::instructions[0xAD] = { "LDA", Absolute, 3, 4, false, opcodes::LDA };
    cpu::CPU::instructions[0xBD] = { "LDA", AbsoluteX, 3, 4, false, opcodes::LDA };
    cpu::CPU::instructions[0xB9] = { "LDA", AbsoluteY, 3, 4, false, opcodes::LDA };
    cpu::CPU::instructions[0xA1] = { "LDA", IdxIndirect, 2, 6, false, opcodes::LDA };
    cpu::CPU::instructions[0xB1] = { "LDA", IndirectIdx, 2, 5, false, opcodes::LDA };

    cpu::CPU::instructions[0xA2] = { "LDX", Immediate, 2, 2, false, opcodes::LDX };
    cpu::CPU::instructions[0xA6] = { "LDX", ZeroPage, 2, 3, false, opcodes::LDX };
    cpu::CPU::instructions[0xB6] = { "LDX", ZeroPageY, 2, 4, false, opcodes::LDX };
    cpu::CPU::instructions[0xAE] = { "LDX", Absolute, 3, 4, false, opcodes::LDX };
    cpu::CPU::instructions[0xBE] = { "LDX", AbsoluteY, 3, 4, false, opcodes::LDX };

    cpu::CPU::instructions[0xA0] = { "LDY", Immediate, 2, 2, false, opcodes::LDY };
    cpu::CPU::instructions[0xA4] = { "LDY", ZeroPage, 2, 3, false, opcodes::LDY };
    cpu::CPU::instructions[0xB4] = { "LDY", ZeroPageX, 2, 4, false, opcodes::LDY };
    cpu::CPU::instructions[0xAC] = { "LDY", Absolute, 3, 4, false, opcodes::LDY };
    cpu::CPU::instructions[0xBC] = { "LDY", AbsoluteX, 3, 4, false, opcodes::LDY };

    cpu::CPU::instructions[0x4A] = { "LSR", Accumulator, 1, 2, false, opcodes::LSR };
    cpu::CPU::instructions[0x46] = { "LSR", ZeroPage, 2, 5, false, opcodes::LSR };
    cpu::CPU::instructions[0x56] = { "LSR", ZeroPageX, 2, 6, false, opcodes::LSR };
    cpu::CPU::instructions[0x4E] = { "LSR", Absolute, 3, 6, false, opcodes::LSR };
    cpu::CPU::instructions[0x5E] = { "LSR", AbsoluteX, 3, 7, false, opcodes::LSR };

    cpu::CPU::instructions[0x1A] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0x3A] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0x5A] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0x7A] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0xDA] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0xFA] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0xEA] = { "NOP", Implicit, 1, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0x80] = { "NOP", Immediate, 2, 2, false, opcodes::NOP };
    cpu::CPU::instructions[0x04] = { "NOP", ZeroPage, 2, 3, false, opcodes::NOP };
    cpu::CPU::instructions[0x44] = { "NOP", ZeroPageX, 2, 3, false, opcodes::NOP };
    cpu::CPU::instructions[0x64] = { "NOP", ZeroPageX, 2, 3, false, opcodes::NOP };
    cpu::CPU::instructions[0x14] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x34] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x54] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x74] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0xD4] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0xF4] = { "NOP", ZeroPageX, 2, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x0C] = { "NOP", Absolute, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x1C] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x3C] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x5C] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x7C] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0xDC] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0xFC] = { "NOP", AbsoluteX, 3, 4, false, opcodes::NOP };
    cpu::CPU::instructions[0x89] = { "NOP", Immediate, 2, 2, false, opcodes::NOP };

    cpu::CPU::instructions[0x09] = { "ORA", Immediate, 2, 2, false, opcodes::ORA };
    cpu::CPU::instructions[0x05] = { "ORA", ZeroPage, 2, 3, false, opcodes::ORA };
    cpu::CPU::instructions[0x15] = { "ORA", ZeroPageX, 2, 4, false, opcodes::ORA };
    cpu::CPU::instructions[0x0D] = { "ORA", Absolute, 3, 4, false, opcodes::ORA };
    cpu::CPU::instructions[0x1D] = { "ORA", AbsoluteX, 3, 4, false, opcodes::ORA };
    cpu::CPU::instructions[0x19] = { "ORA", AbsoluteY, 3, 4, false, opcodes::ORA };
    cpu::CPU::instructions[0x01] = { "ORA", IdxIndirect, 2, 6, false, opcodes::ORA };
    cpu::CPU::instructions[0x11] = { "ORA", IndirectIdx, 2, 5, false, opcodes::ORA };

    cpu::CPU::instructions[0x48] = { "PHA", Implicit, 1, 3, false, opcodes::PHA };
    cpu::CPU::instructions[0x08] = { "PHP", Implicit, 1, 3, false, opcodes::PHP };
    cpu::CPU::instructions[0x68] = { "PLA", Implicit, 1, 4, false, opcodes::PLA };
    cpu::CPU::instructions[0x28] = { "PLP", Implicit, 1, 4, false, opcodes::PLP };

    cpu::CPU::instructions[0x2A] = { "ROL", Accumulator, 1, 2, false, opcodes::ROL };
    cpu::CPU::instructions[0x26] = { "ROL", ZeroPage, 2, 5, false, opcodes::ROL };
    cpu::CPU::instructions[0x36] = { "ROL", ZeroPageX, 2, 6, false, opcodes::ROL };
    cpu::CPU::instructions[0x2E] = { "ROL", Absolute, 3, 6, false, opcodes::ROL };
    cpu::CPU::instructions[0x3E] = { "ROL", AbsoluteX, 3, 7, false, opcodes::ROL };

    cpu::CPU::instructions[0x6A] = { "ROR", Accumulator, 1, 2, false, opcodes::ROR };
    cpu::CPU::instructions[0x66] = { "ROR", ZeroPage, 2, 5, false, opcodes::ROR };
    cpu::CPU::instructions[0x76] = { "ROR", ZeroPageX, 2, 6, false, opcodes::ROR };
    cpu::CPU::instructions[0x6E] = { "ROR", Absolute, 3, 6, false, opcodes::ROR };
    cpu::CPU::instructions[0x7E] = { "ROR", AbsoluteX, 3, 7, false, opcodes::ROR };

    cpu::CPU::instructions[0x40] = { "RTI", Implicit, 1, 6, true, opcodes::RTI };
    cpu::CPU::instructions[0x60] = { "RTS", Implicit, 1, 6, true, opcodes::RTS };

    cpu::CPU::instructions[0xE9] = { "SBC", Immediate, 2, 2, false, opcodes::SBC };
    cpu::CPU::instructions[0xE5] = { "SBC", ZeroPage, 2, 3, false, opcodes::SBC };
    cpu::CPU::instructions[0xF5] = { "SBC", ZeroPageX, 2, 4, false, opcodes::SBC };
    cpu::CPU::instructions[0xED] = { "SBC", Absolute, 3, 4, false, opcodes::SBC };
    cpu::CPU::instructions[0xFD] = { "SBC", AbsoluteX, 3, 4, false, opcodes::SBC };
    cpu::CPU::instructions[0xF9] = { "SBC", AbsoluteY, 3, 4, false, opcodes::SBC };
    cpu::CPU::instructions[0xE1] = { "SBC", IdxIndirect, 2, 6, false, opcodes::SBC };
    cpu::CPU::instructions[0xF1] = { "SBC", IndirectIdx, 2, 5, false, opcodes::SBC };
    cpu::CPU::instructions[0xF2] = { "SBC", ZeroPage, 2, 5, false, opcodes::SBC };


    cpu::CPU::instructions[0x38] = { "SEC", Implicit, 1, 2, false, opcodes::SEC };
    cpu::CPU::instructions[0xF8] = { "SED", Implicit, 1, 2, false, opcodes::SED };
    cpu::CPU::instructions[0x78] = { "SEI", Implicit, 1, 2, false, opcodes::SEI };

    cpu::CPU::instructions[0x85] = { "STA", ZeroPage, 2, 2, false, opcodes::STA };
    cpu::CPU::instructions[0x95] = { "STA", ZeroPageX, 2, 4, false, opcodes::STA };
    cpu::CPU::instructions[0x8D] = { "STA", Absolute, 3, 4, false, opcodes::STA };
    cpu::CPU::instructions[0x9D] = { "STA", AbsoluteX, 3, 5, false, opcodes::STA };
    cpu::CPU::instructions[0x99] = { "STA", AbsoluteY, 3, 5, false, opcodes::STA };
    cpu::CPU::instructions[0x81] = { "STA", IdxIndirect, 2, 6, false, opcodes::STA };
    cpu::CPU::instructions[0x91] = { "STA", IndirectIdx, 2, 6, false, opcodes::STA };

    cpu::CPU::instructions[0x86] = { "STX", ZeroPage, 2, 3, false, opcodes::STX };
    cpu::CPU::instructions[0x96] = { "STX", ZeroPageY, 2, 4, false, opcodes::STX };
    cpu::CPU::instructions[0x8E] = { "STX", Absolute, 3, 4, false, opcodes::STX };

    cpu::CPU::instructions[0x84] = { "STY", ZeroPage, 2, 3, false, opcodes::STY };
    cpu::CPU::instructions[0x94] = { "STY", ZeroPageY, 2, 4, false, opcodes::STY };
    cpu::CPU::instructions[0x8C] = { "STY", Absolute, 3, 4, false, opcodes::STY };

    cpu::CPU::instructions[0xAA] = { "TAX", Implicit, 1, 2, false, opcodes::TAX };
    cpu::CPU::instructions[0xA8] = { "TAY", Implicit, 1, 2, false, opcodes::TAY };
    cpu::CPU::instructions[0xBA] = { "TSX", Implicit, 1, 2, false, opcodes::TSX };
    cpu::CPU::instructions[0x8A] = { "TXA", Implicit, 1, 2, false, opcodes::TXA };
    cpu::CPU::instructions[0x9A] = { "TXS", Implicit, 1, 2, false, opcodes::TXS };
    cpu::CPU::instructions[0x98] = { "TYA", Implicit, 1, 2, false, opcodes::TYA };

    cpu::CPU::instructions[0xA3] = { "LAX", IdxIndirect, 2, 6, false, opcodes::LAX };
    cpu::CPU::instructions[0xA7] = { "LAX", ZeroPage, 2, 3, false, opcodes::LAX };
    cpu::CPU::instructions[0xB7] = { "LAX", ZeroPageY, 2, 4, false, opcodes::LAX };
    cpu::CPU::instructions[0xAF] = { "LAX", Absolute, 3, 4, false, opcodes::LAX };
    cpu::CPU::instructions[0xBF] = { "LAX", AbsoluteY, 3, 4, false, opcodes::LAX };
    cpu::CPU::instructions[0xB3] = { "LAX", IndirectIdx, 2, 5, false, opcodes::LAX };

    cpu::CPU::instructions[0x87] = { "SAX", ZeroPage, 2, 3, false, opcodes::SAX };
    cpu::CPU::instructions[0x97] = { "SAX", ZeroPageY, 2, 4, false, opcodes::SAX };
    cpu::CPU::instructions[0x8F] = { "SAX", Absolute, 3, 4, false, opcodes::SAX };
    cpu::CPU::instructions[0x83] = { "SAX", IdxIndirect, 2, 6, false, opcodes::SAX };

    cpu::CPU::instructions[0xEB] = { "USBC", Immediate, 2, 6, false, opcodes::SBC };

    cpu::CPU::instructions[0xC7] = { "DCP", ZeroPage, 2, 5, false, opcodes::DCP };
    cpu::CPU::instructions[0xD7] = { "DCP", ZeroPageX, 2, 6, false, opcodes::DCP };
    cpu::CPU::instructions[0xCF] = { "DCP", Absolute, 3, 6, false, opcodes::DCP };
    cpu::CPU::instructions[0xDF] = { "DCP", AbsoluteX, 3, 7, false, opcodes::DCP };
    cpu::CPU::instructions[0xDB] = { "DCP", AbsoluteY, 3, 7, false, opcodes::DCP };
    cpu::CPU::instructions[0xC3] = { "DCP", IdxIndirect, 2, 8, false, opcodes::DCP };
    cpu::CPU::instructions[0xD3] = { "DCP", IndirectIdx, 2, 8, false, opcodes::DCP };

    cpu::CPU::instructions[0xE7] = { "ISC", ZeroPage, 2, 5, false, opcodes::ISC };
    cpu::CPU::instructions[0xF7] = { "ISC", ZeroPageX, 2, 6, false, opcodes::ISC };
    cpu::CPU::instructions[0xEF] = { "ISC", Absolute, 3, 6, false, opcodes::ISC };
    cpu::CPU::instructions[0xFF] = { "ISC", AbsoluteX, 3, 7, false, opcodes::ISC };
    cpu::CPU::instructions[0xFB] = { "ISC", AbsoluteY, 3, 7, false, opcodes::ISC };
    cpu::CPU::instructions[0xE3] = { "ISC", IdxIndirect, 2, 8, false, opcodes::ISC };
    cpu::CPU::instructions[0xF3] = { "ISC", IndirectIdx, 2, 8, false, opcodes::ISC };

    cpu::CPU::instructions[0x07] = { "SLO", ZeroPage, 2, 5, false, opcodes::SLO };
    cpu::CPU::instructions[0x17] = { "SLO", ZeroPageX, 2, 6, false, opcodes::SLO };
    cpu::CPU::instructions[0x0F] = { "SLO", Absolute, 3, 6, false, opcodes::SLO };
    cpu::CPU::instructions[0x1F] = { "SLO", AbsoluteX, 3, 7, false, opcodes::SLO };
    cpu::CPU::instructions[0x1B] = { "SLO", AbsoluteY, 3, 7, false, opcodes::SLO };
    cpu::CPU::instructions[0x03] = { "SLO", IdxIndirect, 2, 8, false, opcodes::SLO };
    cpu::CPU::instructions[0x13] = { "SLO", IndirectIdx, 2, 8, false, opcodes::SLO };

    cpu::CPU::instructions[0x27] = { "RLA", ZeroPage, 2, 5, false, opcodes::RLA };
    cpu::CPU::instructions[0x37] = { "RLA", ZeroPageX, 2, 6, false, opcodes::RLA };
    cpu::CPU::instructions[0x2F] = { "RLA", Absolute, 3, 6, false, opcodes::RLA };
    cpu::CPU::instructions[0x3F] = { "RLA", AbsoluteX, 3, 7, false, opcodes::RLA };
    cpu::CPU::instructions[0x3B] = { "RLA", AbsoluteY, 3, 7, false, opcodes::RLA };
    cpu::CPU::instructions[0x23] = { "RLA", IdxIndirect, 2, 8, false, opcodes::RLA };
    cpu::CPU::instructions[0x33] = { "RLA", IndirectIdx, 2, 8, false, opcodes::RLA };

    cpu::CPU::instructions[0x47] = { "SRE", ZeroPage, 2, 5, false, opcodes::SRE };
    cpu::CPU::instructions[0x57] = { "SRE", ZeroPageX, 2, 6, false, opcodes::SRE };
    cpu::CPU::instructions[0x4F] = { "SRE", Absolute, 3, 6, false, opcodes::SRE };
    cpu::CPU::instructions[0x5F] = { "SRE", AbsoluteX, 3, 7, false, opcodes::SRE };
    cpu::CPU::instructions[0x5B] = { "SRE", AbsoluteY, 3, 7, false, opcodes::SRE };
    cpu::CPU::instructions[0x43] = { "SRE", IdxIndirect, 2, 8, false, opcodes::SRE };
    cpu::CPU::instructions[0x53] = { "SRE", IndirectIdx, 2, 8, false, opcodes::SRE };

    cpu::CPU::instructions[0x67] = { "RRA", ZeroPage, 2, 5, false, opcodes::RRA };
    cpu::CPU::instructions[0x77] = { "RRA", ZeroPageX, 2, 6, false, opcodes::RRA };
    cpu::CPU::instructions[0x6F] = { "RRA", Absolute, 3, 6, false, opcodes::RRA };
    cpu::CPU::instructions[0x7F] = { "RRA", AbsoluteX, 3, 7, false, opcodes::RRA };
    cpu::CPU::instructions[0x7B] = { "RRA", AbsoluteY, 3, 7, false, opcodes::RRA };
    cpu::CPU::instructions[0x63] = { "RRA", IdxIndirect, 2, 8, false, opcodes::RRA };
    cpu::CPU::instructions[0x73] = { "RRA", IndirectIdx, 2, 8, false, opcodes::RRA };
}

CPU* cpu::initialize()
{
    CPU* c = new CPU();
    c->A = 0;
    c->X = 0;
    c->Y = 0;
    c->SP = 0xFD;
    c->PS = 0;
    c->PC = 0;

    c->setFlag(CPU::I, 0x1);

    c->populate();

    return c;
}