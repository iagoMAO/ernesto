/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"
#include "headers/cpu//cpu.h"

int main()
{
    std::cout << "[ernesto] - welcome\n";
    
	// Initialize ram & cpu
	memory::initialize();
	cpu::CPU* c = cpu::initialize();

	printf("[ernesto] - RAM ok\n");
	printf("[ernesto] - CPU ok\n");

	// Fill up memory with 0x01
	for (int i = 0; i < memory::ram.size(); i++)
	{
		memory::write((uint16_t)i, 0x00);
	}

	printf("[ernesto] - RAM filled\n");

	// Test instructions
	c->A = 0xAA;
	c->X = 0x01;
	c->Y = 0x0A;
	c->PC = 0x00; // STA opcode

	printf("[ernesto] - Test INC instructions\n");

	for (int i = 0; i < 16; i++)
	{
		cpu::opcodes::INX(*c, cpu::CPU::Implicit);
		cpu::opcodes::INY(*c, cpu::CPU::Implicit);
		printf("[ernesto] CPU - X: 0x%02X | Y: 0x%02X\n", c->X, c->Y);
	}

	printf("[ernesto] - Test DEC instructions\n");

	for (int i = 0; i < 16; i++)
	{
		cpu::opcodes::DEX(*c, cpu::CPU::Implicit);
		cpu::opcodes::DEY(*c, cpu::CPU::Implicit);
		printf("[ernesto] CPU - X: 0x%02X | Y: 0x%02X\n", c->X, c->Y);
	}

	printf("[ernesto] CPU - A: %x | X: %x | Y: %x | PC: %x | SP: %x | PS: %x\n", c->A, c->X, c->Y, c->PC, c->SP, c->PS);

    cin.get();
    return 0;
}