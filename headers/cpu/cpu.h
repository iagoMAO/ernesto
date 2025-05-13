#pragma once
#include <cstdint>
#include <string>
namespace cpu
{
	struct instruction
	{
		std::string name; // name of instruction
		uint8_t size; // size of instruction
		uint8_t cycles; // cycles needed for runtime
		void (*impl)(cpu& cpu); // pointer to implementation
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
}