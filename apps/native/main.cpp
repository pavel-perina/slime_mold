// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif
#define USE_AVX 1

#include "common/colors.h"
#include "common/presets.h"
#include "common/slime_mold.h"
#include "ui_imgui/ui.h"

#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <array>



int main() {
    SlimeMoldSimulation sim;
    Ui ui;
    std::vector<uint8_t> pixels(SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT * 4, 0);

    bool done = false;
    uint64_t last_counter = 0;
    while (!done) {
        sim.step();
#if USE_AVX
        renderToPixelsAvx(pixels);
#else
        renderToPixels(pixels);
#endif
        ui.frame();



        // Handle quit
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT)
                done = true;
        }
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
