/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"

int main()
{
    std::cout << "ernesto\n";
    
    memory::initialize();

    cin.get();
    return 0;
}