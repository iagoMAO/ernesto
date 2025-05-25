/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"
#include "headers/cpu/cpu.h"
#include "headers/rom/rom.h"
#include "headers/gfx/ppu.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL failed to init: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("[ernesto] - ppu",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    if (!window)
    {
        std::cerr << "SDL failed to create window: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    return renderer && texture;
}

int main()
{
    std::cout << "[ernesto] - welcome\n";
    
    // initialize memory
    memory::initialize();

    // load a ROM into memory
    rom::testLoad();

    // initialize CPU
    cpu::CPU* c = cpu::initialize();

    // initialize PPU
    ppu::initialize(*c);

    uint8_t rvL = memory::read(0xFFFC);
    uint8_t rvH = memory::read(0xFFFC + 1);

    uint16_t rv = (rvH << 8) | rvL;

    c->PC = rv;

    if (!initSDL()) return -1;

    bool running = true;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            running = !(e.type == SDL_QUIT);
        }

        uint8_t opcode[3];
        opcode[0] = memory::read(c->PC);
        opcode[1] = memory::read(c->PC + 1);
        opcode[2] = memory::read(c->PC + 2);
        const cpu::CPU::instruction& instr = c->instructions[opcode[0]];

        printf("%04X  %02X %02X %02X  %s  A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
            c->PC,
            opcode[0],
            opcode[1],
            opcode[2],
            instr.name.c_str(),
            c->A,
            c->X,
            c->Y,
            c->PS,
            c->SP);

        if (ppu::frameReady)
        {
            void* pixels;
            int pitch;
            SDL_LockTexture(texture, NULL, &pixels, &pitch);
            memcpy(pixels, ppu::framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
            SDL_UnlockTexture(texture);

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            printf("DRAWING FRAME");
            ppu::frameReady = false;
        }

        if (instr.impl)
        {
            int cycles = instr.cycles;

            instr.impl(*c, instr.mode);

            // tick PPU 3 times
            for (int i = 0; i < cycles * 3; ++i)
                ppu::tick();

            if (!instr.incrementPc)
                c->PC += instr.size;
        }
        else
        {
            printf("\n[ernesto] - unimplemented opcode: %02X", opcode[0]);
        }
    }

    cin.get();
    return 0;
}