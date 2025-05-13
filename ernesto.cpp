/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"

int main()
{
    std::cout << "ernesto\n";
    std::cout << "ram check\n";
    
    memory::initialize();

    // fill up memory with 0xCC
    for (int i = 0; i < 0x800; i++)
    {
        memory::write(i, 0xCC);
    }

    cin.get();
    return 0;
}