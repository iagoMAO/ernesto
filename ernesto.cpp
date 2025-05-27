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

    // initialize PPU
    ppu::initialize(*c);

    uint8_t rvL = memory::read(0xFFFC);
    uint8_t rvH = memory::read(0xFFFD);

    uint16_t rv = (rvH << 8) | rvL;

    c->PC = rv;

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

        if (ppu::frameReady)
        {
            void* pixels;
            int pitch;
            if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0)
            {
                memcpy(pixels, ppu::framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
                SDL_UnlockTexture(texture);
            }
            else
                printf("SDL_LockTexture failed: %s\n", SDL_GetError());

            ppu::frameReady = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

        char buf[256];
        snprintf(buf, sizeof(buf), "%04X -  %02X %02X %02X  %s\n",
            c->PC,
            opcode[0],
            opcode[1],
            opcode[2],
            instr.name.c_str());

        log.push_back(buf);
        if (log.size() > 1000) log.erase(log.begin());

        ImGui::Begin("[ernesto] - instructions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        for (const auto& line : log)
            ImGui::TextUnformatted(line.c_str());

        ImGui::SetScrollHereY(1.0f);

        ImGui::End();

        static MemoryEditor pattern;
        pattern.DrawWindow("[ernesto] - ppu patterns", ppu::pattern_tables.data(), ppu::pattern_tables.size());

        static MemoryEditor nametables;
        nametables.DrawWindow("[ernesto] - ppu nametables", ppu::nametables.data(), ppu::nametables.size());

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
            bool bit = (*ppu::ppu_ctrl >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("PPU_MASK: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (*ppu::ppu_mask >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("PPU_STATUS: ");
        for (int i = 7; i >= 0; --i)
        {
            bool bit = (*ppu::ppu_status >> i) & 1;
            ImGui::SameLine();
            ImGui::Text("%d", bit);
        }
        ImGui::Text("OAM_ADDR: %02X", *ppu::oam_addr);
        ImGui::Text("OAM_DATA: %02X", *ppu::oam_data);
        ImGui::Text("PPU_SCROLL: %04X", *ppu::ppu_scroll);
        ImGui::Text("PPU_ADDR: %04X", *ppu::ppu_addr);
        ImGui::Text("PPU_DATA: %02X", *ppu::ppu_data);
        ImGui::Text("OAM_DMA: %02X", *ppu::oam_dma);
        ImGui::End();

        ImGui::Begin("[ernesto] - display", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image((ImTextureID)texture, ImVec2(256, 240));
        ImGui::End();

        ImGui::Render();

        SDL_RenderClear(renderer);

        // SDL_RenderCopy(renderer, texture, NULL, NULL);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

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
            continue;
        }
    }

    cin.get();
    return 0;
}