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

		struct instruction
		{
			std::string name; // name of instruction
			uint8_t size; // size of instruction
			uint8_t cycles; // cycles needed for runtime
			void (*impl)(CPU& cpu); // pointer to implementation
		};

		void setFlag(flags flag, bool value);
		bool getFlag(flags flag) const;
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
		uint8_t& accumulator(CPU& c);
		void implied(CPU& c);

		uint16_t cpu::addressing::resolve(CPU& c, CPU::addressingMode mode);
	}

	namespace opcodes
	{
		// Access instructions
		void LDA(CPU& c, CPU::addressingMode mode);
		void STA(CPU& c, CPU::addressingMode mode);
		void LDX(CPU& c, CPU::addressingMode mode);
		void STX(CPU& c, CPU::addressingMode mode);
		void LDY(CPU& c, CPU::addressingMode mode);
		void STY(CPU& c, CPU::addressingMode mode);

		// Transfer instructions
		void TAX(CPU& c, CPU::addressingMode mode);
		void TXA(CPU& c, CPU::addressingMode mode);
		void TAY(CPU& c, CPU::addressingMode mode);
		void TYA(CPU& c, CPU::addressingMode mode);

		// Arithmetic instructions
		void ADC(CPU& c, CPU::addressingMode mode);
		void SBC(CPU& c, CPU::addressingMode mode);
		void INC(CPU& c, CPU::addressingMode mode);
		void DEC(CPU& c, CPU::addressingMode mode);
		void INX(CPU& c, CPU::addressingMode mode);
		void DEX(CPU& c, CPU::addressingMode mode);
		void INY(CPU& c, CPU::addressingMode mode);
		void DEY(CPU& c, CPU::addressingMode mode);

		// Shift instructions
		void ASL(CPU& c, CPU::addressingMode mode);
		void LSR(CPU& c, CPU::addressingMode mode);
		void ROL(CPU& c, CPU::addressingMode mode);
		void ROR(CPU& c, CPU::addressingMode mode);

		// Bitwise instructions
		void AND(CPU& c, CPU::addressingMode mode);
		void ORA(CPU& c, CPU::addressingMode mode);
		void EOR(CPU& c, CPU::addressingMode mode);
		void BIT(CPU& c, CPU::addressingMode mode);

		// Compare instructions
		void CMP(CPU& c, CPU::addressingMode mode);
		void CPX(CPU& c, CPU::addressingMode mode);
		void CPY(CPU& c, CPU::addressingMode mode);

		// Branch instructions
		void BCC(CPU& c, CPU::addressingMode mode);
		void BCS(CPU& c, CPU::addressingMode mode);
		void BEQ(CPU& c, CPU::addressingMode mode);
		void BNE(CPU& c, CPU::addressingMode mode);
		void BPL(CPU& c, CPU::addressingMode mode);
		void BMI(CPU& c, CPU::addressingMode mode);
		void BVC(CPU& c, CPU::addressingMode mode);
		void BVS(CPU& c, CPU::addressingMode mode);
		
		// Jump instructions
		void JMP(CPU& c, CPU::addressingMode mode);
		void JSR(CPU& c, CPU::addressingMode mode);
		void RTS(CPU& c, CPU::addressingMode mode);
		void BRK(CPU& c, CPU::addressingMode mode);
		void RTI(CPU& c, CPU::addressingMode mode);

		// Stack instructions
		void PHA(CPU& c, CPU::addressingMode mode);
		void PLA(CPU& c, CPU::addressingMode mode);
		void PHP(CPU& c, CPU::addressingMode mode);
		void PLP(CPU& c, CPU::addressingMode mode);
		void TXS(CPU& c, CPU::addressingMode mode);
		void TSX(CPU& c, CPU::addressingMode mode);

		// Flag instructions
		void CLC(CPU& c, CPU::addressingMode mode);
		void SEC(CPU& c, CPU::addressingMode mode);
		void CLI(CPU& c, CPU::addressingMode mode);
		void SEI(CPU& c, CPU::addressingMode mode);
		void CLD(CPU& c, CPU::addressingMode mode);
		void SED(CPU& c, CPU::addressingMode mode);
		void CLV(CPU& c, CPU::addressingMode mode);

		// nop
		void NOP(CPU& c, CPU::addressingMode mode);
	}

	void populate();
	void initialize();
}