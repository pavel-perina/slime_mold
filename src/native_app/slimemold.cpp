// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif
#define USE_AVX 1

#include "colors.h"
#include "presets.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <array>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// WARNING: WIDTH*HEIGHT must be divisible by 8 due to vectorization code
constexpr int   SIDEPANEL_WIDTH = 224;
constexpr int   TOTAL_WIDTH = SIDEPANEL_WIDTH + WIDTH;
constexpr size_t PALETTE_SIZE = 1024;



int selectedPreset   = 0;
int selectedPalette = 0;
const std::vector<AgentPreset>   presets  = presetAgents();
const std::vector<PalettePreset> palettes = presetPalettes();
enum CMapInterpolation
{
    CMAP_INTERP_RGB,
    CMAP_INTERP_LAB,
    CMAP_INTERP_LCH,
};
std::array<std::string, 3> cmapLabels = { "RGB", "LAB", "LCH" };
int cmapInterpolation = CMAP_INTERP_LCH;






std::vector<uint8_t> preparePalette()
{
    size_t mid = (size_t)(palette_mid * PALETTE_SIZE);
    GradientFunction gradientFn = nullptr;
    switch (cmapInterpolation)
    {
    case CMAP_INTERP_RGB:
        gradientFn = RGBGradient;
        break;
    case CMAP_INTERP_LAB:
        gradientFn = LABGradient;
        break;
    case CMAP_INTERP_LCH:
    default:
        gradientFn = LCHGradient;
    }
    auto g1 = gradientFn(paletteA, paletteB, mid);
    auto g2 = gradientFn(paletteB, paletteC, PALETTE_SIZE - mid);
    std::vector<uint8_t> palette(PALETTE_SIZE * 4);
    for (size_t i = 0; i < PALETTE_SIZE; i++) {
        palette[i * 4] = 255.0f;
        if (i < mid) {
            palette[i * 4 + 1] = g1[i].r * 255.0f;
            palette[i * 4 + 2] = g1[i].g * 255.0f;
            palette[i * 4 + 3] = g1[i].b * 255.0f;
        }
        else {
            size_t j = i - mid;
            palette[i * 4 + 1] = g2[j].r * 255.0f;
            palette[i * 4 + 2] = g2[j].g * 255.0f;
            palette[i * 4 + 3] = g2[j].b * 255.0f;
        }
    }
    return palette;
}


void renderToPixels(std::vector<uint8_t>& pixels)
{
    const auto palette = preparePalette();
    constexpr float k = 10.0f * PALETTE_SIZE / 256.0f;
    const uint32_t* paletteU32 = reinterpret_cast<const uint32_t*>(palette.data());
    uint32_t* pixelsU32 = reinterpret_cast<uint32_t*>(pixels.data());
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        int c = std::min(field[i] * k, static_cast<float>(PALETTE_SIZE - 1));
        //uint8_t c = (uint8_t)std::min(log(field[i]+2.73f)*20.f, 255.0f);
        pixelsU32[i] = paletteU32[c];
    }
}


void renderToPixelsAvx(std::vector<uint8_t>& pixels)
{
    const auto palette = preparePalette();

    // Initialize scale and clamp
    const __m256 kVec   = _mm256_set1_ps(10.0f * PALETTE_SIZE / 256.0f);
    const __m256 maxIdx = _mm256_set1_ps(static_cast<float>(PALETTE_SIZE - 1));

    // Process 8 pixels at a time
    constexpr size_t avxWidth = 8;
    const float* fieldData = field.data();
    for (size_t i = 0; i < WIDTH * HEIGHT; i += avxWidth) {
        // Load 8 field values
        __m256 fieldVals = _mm256_loadu_ps(fieldData +i);
        // Scale and clamp
        fieldVals = _mm256_mul_ps(fieldVals, kVec);
        fieldVals = _mm256_min_ps(fieldVals, maxIdx);
        // Convert to integers (palette indices)
        __m256i indices = _mm256_cvtps_epi32(fieldVals);
        // Fetch colors as uint32_t
        __m256i colors = _mm256_i32gather_epi32(
            reinterpret_cast<const int*>(palette.data()),   // base pointer (cast to int*)
            indices,                                        // the 8 indices
            4                                               // scale: each index * 4 bytes
        );
        // Store colors to pixels
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(pixels.data() + i * 4), colors);
    }
}



void SlimeMoldSimulation::resetAgents() {
    srand((unsigned)time(0));
    for (auto& a : agents) {
        a.x = rand() % WIDTH;
        a.y = rand() % HEIGHT;
        float angle = (rand() / (float)RAND_MAX) * 2.0f * M_PI;
        a.dx = std::cos(angle);
        a.dy = std::sin(angle);
    }
}

void SlimeMoldSimulation::updateAgent(int idx) {
    sensor_angle = presets[idx].sensor_angle;
    sensor_dist  = presets[idx].sensor_dist;
    evaporate    = presets[idx].evaporate;
    step_size    = presets[idx].step_size;
    turn_angle   = presets[idx].turn_angle;
    palette_mid  = presets[idx].palette_mid;
}

void SlimeMoldSimulation::updatePalette(int idx)
{
    paletteA = palettes[idx].paletteA;
    paletteB = palettes[idx].paletteB;
    paletteC = palettes[idx].paletteC;
    // TODO: update LUT here
}

int main() {
    SlimeMoldSimulation sim;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window   *window   = SDL_CreateWindow("Slime Mold", TOTAL_WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    SDL_Texture  *texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 4, 0);

    bool done = false;
    uint64_t last_counter = 0;
    while (!done) {
        updateAgents();
#if USE_AVX
        diffuseAvx();
        renderToPixelsAvx(pixels);
#else
        diffuse();
        renderToPixels(pixels);
#endif

        // Prepare a new frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        uint64_t current_counter = SDL_GetPerformanceCounter();
        float delta_time = (current_counter - last_counter) / (float)SDL_GetPerformanceFrequency();
        float fps = 1.0f / delta_time;
        last_counter = current_counter;

        // Position the sidebar on the right side
        ImGui::SetNextWindowPos(ImVec2(WIDTH, 0));
        ImGui::SetNextWindowSize(ImVec2(SIDEPANEL_WIDTH, HEIGHT));
        ImGui::Begin("Parameters", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar);
        ImGui::PushItemWidth(-1); // Use full available width for sliders
        ImGui::Spacing();
        ImGui::Text("FPS %.1f", fps);

        ImGui::Text("Simulation Parameters");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Sensor Angle");
        ImGui::SliderFloat("##sensor_angle", &sensor_angle, 0.0f, 2.0f);
        ImGui::Text("Sensor Distance");
        ImGui::SliderFloat("##sensor_dist", &sensor_dist, 1.0f, 12.0f);
        ImGui::Text("Turn Angle");
        ImGui::SliderFloat("##turn_angle", &turn_angle, 0.0f, 1.0f);
        ImGui::Text("Step Size");
        ImGui::SliderFloat("##step_size", &step_size, 0.1f, 5.0f);
        ImGui::Text("Evaporation");
        ImGui::SliderFloat("##evaporate", &evaporate, 0.5f, 0.99f);
        ImGui::Spacing();
        if (ImGui::Button("Reset")) {
            resetAgents();
            clearField();
        }
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Color Palette");
        ImGui::ColorEdit3("Start",    &paletteA.r, ImGuiColorEditFlags_NoLabel);
        ImGui::ColorEdit3("Midpoint", &paletteB.r, ImGuiColorEditFlags_NoLabel);
        ImGui::ColorEdit3("Endpoint", &paletteC.r, ImGuiColorEditFlags_NoLabel);
        ImGui::Text("Palette Midpoint");
        ImGui::SliderFloat("##palette_mid", &palette_mid, 0.0f, 1.0f);
#if 0
        // Maybe hide this, LCH is superior and least boring
        ImGui::Text("Color interpolation");
        ImGui::Columns(3, "##cmap_interp", false);
        ImGui::RadioButton("RGB", &cmapInterpolation, 0);  ImGui::NextColumn();
        ImGui::RadioButton("LAB", &cmapInterpolation, 1);  ImGui::NextColumn();
        ImGui::RadioButton("LCH", &cmapInterpolation, 2);
        ImGui::Columns(1);
#endif
        ImGui::Separator();
        if (ImGui::BeginCombo("##Behavior", presets[selectedPreset].name.c_str())) {
            for (int i = 0; i < presets.size(); i++) {
                bool is_selected = (selectedPreset == i);
                if (ImGui::Selectable(presets[i].name.c_str(), is_selected)) {
                    selectedPreset = i;
                    updateAgent(selectedPreset);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("##Palette", palettes[selectedPalette].name.c_str())) {
            for (int i = 0; i < palettes.size(); i++) {
                bool is_selected = (selectedPalette == i);
                if (ImGui::Selectable(palettes[i].name.c_str(), is_selected)) {
                    selectedPalette = i;
                    updatePalette(selectedPalette);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
        ImGui::End();

        ImGui::Render();

        // Clear canvas with a background color (dark gray background)
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        // Define the destination rectangle for the main simulation area
        const SDL_FRect mainRect = { 0, 0, WIDTH, HEIGHT };

        // Upload pixel data and render simulation
        SDL_UpdateTexture(texture, nullptr, pixels.data(), WIDTH * 4);
        SDL_RenderTexture(renderer, texture, nullptr, &mainRect);

        // Render ImGui on top
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

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
