#include "pch.h"
#include "CppUnitTest.h"
#include "../../headers/mem/ram.h"
#include "../../headers/cpu/cpu.h"
#include "../../cpu/cpu.cpp"
#include "../../mem/ram.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace cpuTest
{
	TEST_CLASS(cpuTest)
	{
	public:
		
		TEST_METHOD(cpuTest01)
		{
			// Test some instructions
			memory::initialize();
			
			// Fill up memory with 0x01
			for (int i = 0; i < memory::ram.size(); i++)
			{
				memory::write((uint16_t)i, 0x01);
			}

			printf("Memory filled");
		}
	};
}
