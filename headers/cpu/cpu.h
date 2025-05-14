#pragma once
#include <cstdint>
#include <string>
namespace cpu
{
	struct CPU
	{
		uint8_t A; // Accumulator
		uint8_t X; // Index register X
		uint8_t Y; // Index register Y
		uint8_t SP; // Stack pointer 
		uint8_t PS; // Processor status
		uint16_t PC; // Program Counter

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

		void setFlag(flags flag, bool value);
		bool getFlag(flags flag) const;
	};

	struct instruction
	{
		std::string name; // name of instruction
		uint8_t size; // size of instruction
		uint8_t cycles; // cycles needed for runtime
		void (*impl)(CPU& cpu); // pointer to implementation
	};

	enum addressingMode
	{
		Accumulator, 
		Immediate, 
		ZeroPage, 
		ZeroPageX, 
		ZeroPageY, 
		Relative, 
		Absolute, 
		AbsoluteX, 
		AbsoluteY, 
		Indirect, 
		IdxIndirect, 
		IndirectIdx
	};

	namespace addressing
	{
		uint16_t immediate(CPU& c);
		uint16_t zeroPage(CPU& c);
		uint16_t zeroPageX(CPU& c);
		uint16_t zeroPageY(CPU& c);
		uint16_t absolute(CPU& c);
		uint16_t absoluteX(CPU& c);
		uint16_t absoluteY(CPU& c);
		uint16_t indirect(CPU& c);
		uint16_t indirectX(CPU& c);
		uint16_t indirectY(CPU& c);
		int16_t relative(CPU& c);
		void implied(CPU& c);
		uint8_t& accumulator(CPU& c);
	}

	void populate();
	void initialize();
}