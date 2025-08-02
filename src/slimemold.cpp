// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

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
constexpr int   NUM_AGENTS = 250000;

float sensor_angle = 0.5f;
float sensor_dist = 5.0f;
float turn_angle = 0.3f;
float step_size = 1.0f;
float evaporate = 0.99f;


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

void renderToPixels(std::vector<uint8_t> &pixels) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        uint8_t c = (uint8_t)std::min(field[i] * 10.0f, 255.0f);
        pixels[i * 3 + 0] = c; // Blue
        pixels[i * 3 + 1] = 0; // Green
        pixels[i * 3 + 2] = c; // Red
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

int main() {
    resetAgents();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Slime Mold", WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);


    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3, 0);

    bool done = false;
    while (!done) {
        updateAgents();
        diffuse();
        renderToPixels(pixels);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Parameters");
        ImGui::SliderFloat("Sensor Angle", &sensor_angle, 0.0f, 2.0f);
        ImGui::SliderFloat("Sensor Distance", &sensor_dist, 1.0f, 20.0f);
        ImGui::SliderFloat("Turn Angle", &turn_angle, 0.0f, 1.0f);
        ImGui::SliderFloat("Step Size", &step_size, 0.1f, 5.0f);
        ImGui::SliderFloat("Evaporation", &evaporate, 0.5f, 0.99f);
        if (ImGui::Button("Reset Agents")) {
            resetAgents();
        }
        ImGui::End();

        ImGui::Render();

        // Update window
        SDL_UpdateTexture(texture, NULL, pixels.data(), WIDTH * 3);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        /*
        // Save every 10 frames
        if (frame % 10 == 0) {
            char filename[64];
            sprintf(filename, "frame_%04d.tga", frame);
            saveTGA(filename, pixels);
        }*/

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
