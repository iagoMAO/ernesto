/*
    ernesto - 6502, ergo NES emulator
    author: Iago Maldonado (@iagoMAO)
*/

#include <iostream>
#include "headers/mem/ram.h"
#include "headers/cpu/cpu.h"
#include "headers/rom/rom.h"
#include "headers/gfx/ppu.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "headers/third_party/imgui_club/imgui_memory_editor.h"

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
        SCREEN_WIDTH * 3, SCREEN_HEIGHT * 3, SDL_WINDOW_RESIZABLE);
    
    if (!window)
    {
        std::cerr << "SDL failed to create window: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

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

    uint8_t rvL = memory::read(0xFFFC);
    uint8_t rvH = memory::read(0xFFFD);

    uint16_t rv = (rvH << 8) | rvL;

    // c->PC = rv;
    c->PS = 0x24;
    c->PC = 0xC000;

    if (!initSDL()) return -1;

    bool running = true;
    SDL_Event e;

    std::vector<std::string> log;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL2_ProcessEvent(&e);
            running = !(e.type == SDL_QUIT);
        }

        uint8_t opcode[3];
        opcode[0] = memory::read(c->PC);
        opcode[1] = memory::read(c->PC + 1);
        opcode[2] = memory::read(c->PC + 2);
        const cpu::CPU::instruction& instr = c->instructions[opcode[0]];

        /* printf("%04X  %02X %02X %02X  %s  A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
            c->PC,
            opcode[0],
            opcode[1],
            opcode[2],
            instr.name.c_str(),
            c->A,
            c->X,
            c->Y,
            c->PS,
            c->SP); */

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

        char buf[256];
        snprintf(buf, sizeof(buf), "%04X  %02X %02X %02X  %s %02X %02X  A: %02X X: %02X Y: %02X P: %02X SP: %02X\n",
            c->PC,
            opcode[0],
            opcode[1],
            opcode[2],
            instr.name.c_str(),
            opcode[1],
            opcode[2],
            c->A,
            c->X,
            c->Y,
            c->PS,
            c->SP);

        log.push_back(buf);

        ImGui::Begin("[ernesto] - instructions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        for (const auto& line : log)
            ImGui::TextUnformatted(line.c_str());

        // ImGui::SetScrollHereY(1.0f);

        ImGui::End();

        ImGui::Begin("[ernesto] - cpu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("A: %02X", c->A);
        ImGui::Text("X: %02X", c->X);
        ImGui::Text("Y: %02X", c->Y);
        ImGui::Text("SP: %02X", c->SP);
        ImGui::Text("PC: %02X", c->PC);
        ImGui::Text("PS: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (c->PS >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::End();

        ImGui::Begin("[ernesto] - ppu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("PPU_CTRL: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (0 >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("PPU_MASK: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (0 >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("PPU_STATUS: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (0 >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("OAM_ADDR: %02X", 0);
        ImGui::Text("OAM_DATA: %02X", 0);
        ImGui::Text("PPU_SCROLL: %04X", 0);
        ImGui::Text("PPU_ADDR: %04X", 0);
        ImGui::Text("PPU_DATA: %02X", 0);
        ImGui::Text("OAM_DMA: %02X", 0);
        ImGui::End();

        ImGui::Begin("[ernesto] - display", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((ImTextureID)texture, ImVec2(256, 240));
        ImGui::End();

        ImGui::Render();

        SDL_RenderClear(renderer);

        // SDL_RenderCopy(renderer, texture, NULL, NULL);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        uint8_t oldPc;

        if (c->PC == 0xC657)
            printf("FUCJ");

        if (instr.impl)
        {
            oldPc = c->PC;

            int cycles = instr.cycles;

            instr.impl(*c, instr.mode);

            if (!instr.incrementPc)
                c->PC += instr.size;
        }
        else
        {
            printf("\n[ernesto] - unimplemented opcode: %02X", opcode[0]);
            continue;
        }
    }

    cin.get();
    return 0;
}