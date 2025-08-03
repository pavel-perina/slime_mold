// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

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
#include <cstring>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr int   WIDTH = 640;
constexpr int   HEIGHT = 480;
constexpr int   SIDEPANEL_WIDTH = 224;
constexpr int   TOTAL_WIDTH = SIDEPANEL_WIDTH + WIDTH;
constexpr int   NUM_AGENTS = 250000;

float sensor_angle = 0.5f;
float sensor_dist  = 5.0f;
float turn_angle   = 0.3f;
float step_size    = 1.0f;
float evaporate    = 0.95f;

int selectedPreset   = 0;
int selectedPalette = 0;
const std::vector<AgentPreset>   presets  = presetAgents();
const std::vector<PalettePreset> palettes = presetPalettes();


ColorRGB paletteA = { 0.31f, 0.14f, 0.33f };
ColorRGB paletteB = { 0.87f, 0.85f, 0.65f };
ColorRGB paletteC = { 0.54f, 0.99f, 0.77f };
float palette_mid = 0.5f;

struct Agent {
    float x, y, dx, dy;
};

std::vector<Agent> agents(NUM_AGENTS);
std::vector<float> field(WIDTH * HEIGHT, 0.0f);

inline int idx(int x, int y) {
    return (y % HEIGHT) * WIDTH + (x % WIDTH);
}

float sampleField(float x, float y) {
    int xi = ((int)(x+0.5f) + WIDTH) % WIDTH;
    int yi = ((int)(y+0.5f) + HEIGHT) % HEIGHT;
    return field[idx(xi, yi)];
}

void deposit(Agent &a) {
    int xi = ((int)(a.x+0.5f) + WIDTH) % WIDTH;
    int yi = ((int)(a.y+0.5f) + HEIGHT) % HEIGHT;
    field[idx(xi, yi)] += 1.0f;
}

inline void rotate(float& dx, float& dy, float cos_a, float sin_a) {
    float ndx = dx * cos_a - dy * sin_a;
    float ndy = dx * sin_a + dy * cos_a;
    dx = ndx;
    dy = ndy;
}

void updateAgents() {
    const float SENSOR_LEFT_COS = std::cos(-sensor_angle);
    const float SENSOR_LEFT_SIN = std::sin(-sensor_angle);
    const float SENSOR_RIGHT_COS = std::cos(sensor_angle);
    const float SENSOR_RIGHT_SIN = std::sin(sensor_angle);

    const float TURN_LEFT_COS = std::cos(-turn_angle);
    const float TURN_LEFT_SIN = std::sin(-turn_angle);
    const float TURN_RIGHT_COS = std::cos(turn_angle);
    const float TURN_RIGHT_SIN = std::sin(turn_angle);

    for (auto &a : agents) {
        // Sensor positions
        float cx = a.x + a.dx * sensor_dist;
        float cy = a.y + a.dy * sensor_dist;

        float ldx = a.dx * SENSOR_LEFT_COS - a.dy * SENSOR_LEFT_SIN;
        float ldy = a.dx * SENSOR_LEFT_SIN + a.dy * SENSOR_LEFT_COS;
        float lx = a.x + ldx * sensor_dist;
        float ly = a.y + ldy * sensor_dist;

        float rdx = a.dx * SENSOR_RIGHT_COS - a.dy * SENSOR_RIGHT_SIN;
        float rdy = a.dx * SENSOR_RIGHT_SIN + a.dy * SENSOR_RIGHT_COS;
        float rx = a.x + rdx * sensor_dist;
        float ry = a.y + rdy * sensor_dist;

        // Sample sensors
        float c = sampleField(cx, cy);
        float l = sampleField(lx, ly);
        float r = sampleField(rx, ry);


        // Adjust angle
        if (c > l && c > r) {
            // keep direction
        }
        else if (l > r) {
            rotate(a.dx, a.dy, TURN_LEFT_COS, TURN_LEFT_SIN);
        }
        else if (r > l) {
            rotate(a.dx, a.dy, TURN_RIGHT_COS, TURN_RIGHT_SIN);
        }
        else {
            if (rand() % 2) {
                rotate(a.dx, a.dy, TURN_LEFT_COS, TURN_LEFT_SIN);
            }
            else {
                rotate(a.dx, a.dy, TURN_RIGHT_COS, TURN_RIGHT_SIN);
            }
        }

        // Move
        a.x += a.dx * step_size;
        a.y += a.dy * step_size;

        // Wrap around
        if (a.x < 0) a.x += WIDTH;
        if (a.x >= WIDTH) a.x -= WIDTH;
        if (a.y < 0) a.y += HEIGHT;
        if (a.y >= HEIGHT) a.y -= HEIGHT;

        deposit(a);
    }
}

void diffuse() {
    // Evaporation only for simplicity
    for (auto &v : field) v *= evaporate;
}

void clearField() {
    for (auto& v : field) v = 0.0f;
}

void renderToPixels(std::vector<uint8_t> &pixels) {
    // Prepare palette
    constexpr size_t PALETTE_SIZE = 1024;
    size_t mid = (size_t)(palette_mid * PALETTE_SIZE);
    auto g1 = LCHGradient2(paletteA, paletteB, mid);
    auto g2 = LCHGradient2(paletteB, paletteC, PALETTE_SIZE - mid);
    std::vector<uint8_t> palette(PALETTE_SIZE * 3);
    for (size_t i = 0; i < PALETTE_SIZE; i++) {
        if (i < mid) {
            palette[i * 3]     = g1[i].r * 255.0f;
            palette[i * 3 + 1] = g1[i].g * 255.0f;
            palette[i * 3 + 2] = g1[i].b * 255.0f;
        }
        else {
            size_t j = i - mid;
            palette[i * 3]     = g2[j].r * 255.0f;
            palette[i * 3 + 1] = g2[j].g * 255.0f;
            palette[i * 3 + 2] = g2[j].b * 255.0f;
        }
    }

    constexpr float k = 10.0f * PALETTE_SIZE / 256.0f;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        int c = std::min(field[i] * k , static_cast<float>(PALETTE_SIZE-1));
        //uint8_t c = (uint8_t)std::min(log(field[i]+2.73f)*20.f, 255.0f);
        pixels[i * 3 + 0] = palette[c * 3 + 0];
        pixels[i * 3 + 1] = palette[c * 3 + 1];
        pixels[i * 3 + 2] = palette[c * 3 + 2];
    }
}

void saveTGA(const char *filename, const std::vector<uint8_t> &pixels) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    uint8_t header[18] = {};
    header[2] = 2; // uncompressed true-color
    header[12] = WIDTH & 0xFF;
    header[13] = (WIDTH >> 8) & 0xFF;
    header[14] = HEIGHT & 0xFF;
    header[15] = (HEIGHT >> 8) & 0xFF;
    header[16] = 24; // bits per pixel
    fwrite(header, 1, 18, f);
    fwrite(pixels.data(), 1, pixels.size(), f);
    fclose(f);
}

void resetAgents() {
    srand((unsigned)time(0));
    for (auto& a : agents) {
        a.x = rand() % WIDTH;
        a.y = rand() % HEIGHT;
        float angle = (rand() / (float)RAND_MAX) * 2.0f * M_PI;
        a.dx = std::cos(angle);
        a.dy = std::sin(angle);
    }
}

void updateAgent(int idx) {
    sensor_angle = presets[idx].sensor_angle;
    sensor_dist  = presets[idx].sensor_dist;
    evaporate    = presets[idx].evaporate;
    step_size    = presets[idx].step_size;
    turn_angle   = presets[idx].turn_angle;
    palette_mid  = presets[idx].palette_mid;
}

void updatePalette(int idx)
{
    paletteA = palettes[idx].paletteA;
    paletteB = palettes[idx].paletteB;
    paletteC = palettes[idx].paletteC;

}

int main() {
    resetAgents();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window   *window   = SDL_CreateWindow("Slime Mold", TOTAL_WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture  *texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3, 0);

    bool done = false;
    uint64_t last_counter = 0;
    while (!done) {
        updateAgents();
        diffuse();
        renderToPixels(pixels);

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
        ImGui::ColorEdit3("Start", &paletteA.r, ImGuiColorEditFlags_NoLabel);
        ImGui::ColorEdit3("Midpoint", &paletteB.r, ImGuiColorEditFlags_NoLabel);
        ImGui::ColorEdit3("Endpoint", &paletteC.r, ImGuiColorEditFlags_NoLabel);
        ImGui::Text("Palette Midpoint");
        ImGui::SliderFloat("##palette_mid", &palette_mid, 0.0f, 1.0f);

        ImGui::Separator();
        if (ImGui::BeginCombo("Behavior", presets[selectedPreset].name.data())) {
            for (int i = 0; i < presets.size(); i++) {
                bool is_selected = (selectedPreset == i);
                if (ImGui::Selectable(presets[i].name.data(), is_selected)) {
                    selectedPreset = i;
                    updateAgent(selectedPreset);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Palette", palettes[selectedPalette].name.data())) {
            for (int i = 0; i < palettes.size(); i++) {
                bool is_selected = (selectedPalette == i);
                if (ImGui::Selectable(palettes[i].name.data(), is_selected)) {
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
        SDL_UpdateTexture(texture, nullptr, pixels.data(), WIDTH * 3);
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
