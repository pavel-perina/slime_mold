//! \file ui.cpp
#include "ui_imgui/ui.h"
#include "common/slime_mold_viewmodel.h"
// FIXME: holds width, height globals
#include "common/slime_mold_simulation.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <string>
#include <array>

// TODO: initialization error handling
// TODO: separate logic (agent, palette) from ImGui

class Ui::Private {
public:
    //! Constructor
    Private();

    // SDL stuff
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    bool done = false;
    bool initialized = false;

    SlimeMoldViewModel viewModel;
    std::vector<uint8_t> pixels;

    uint64_t last_counter = 0;
};


Ui::Private::Private()
{
    pixels.resize(SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT * 4);
}


Ui::Ui()
    : m_p(std::make_unique<Private>())
{
    constexpr size_t TOTAL_WIDTH = 224 + SlimeMoldSimulation::WIDTH;
    SDL_Init(SDL_INIT_VIDEO);
    m_p->window = SDL_CreateWindow("Slime Mold", TOTAL_WIDTH, SlimeMoldSimulation::HEIGHT, 0);
    if (!m_p->window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return;
    }
    m_p->renderer = SDL_CreateRenderer(m_p->window, nullptr);
    m_p->texture = SDL_CreateTexture(
        m_p->renderer,
        SDL_PIXELFORMAT_ARGB32,
        SDL_TEXTUREACCESS_STREAMING,
        SlimeMoldSimulation::WIDTH,
        SlimeMoldSimulation::HEIGHT
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(m_p->window, m_p->renderer);
    ImGui_ImplSDLRenderer3_Init(m_p->renderer);
    m_p->initialized = true;
}


Ui::~Ui()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(m_p->texture);
    SDL_DestroyRenderer(m_p->renderer);
    SDL_DestroyWindow(m_p->window);
    SDL_Quit();
}


bool Ui::initialized()
{
    return m_p->initialized;
}


bool Ui::done()
{
    return m_p->done;
}


void Ui::frame() 
{
    // Do simulation step
    /*m_p->sim.step(m_p->agent);
    m_p->renderToPixels(m_p->pixels, m_p->sim.data());*/
    auto &vm = m_p->viewModel;
    vm.updatePixels(m_p->pixels.data());

    // Prepare a new frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    uint64_t current_counter = SDL_GetPerformanceCounter();
    float delta_time = (current_counter - m_p->last_counter) / (float)SDL_GetPerformanceFrequency();
    float fps = 1.0f / delta_time;
    m_p->last_counter = current_counter;

    //AgentPreset& agent = m_p->agent;

    // Position the sidebar on the right side
    ImGui::SetNextWindowPos(ImVec2(SlimeMoldSimulation::WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(SIDEPANEL_WIDTH, SlimeMoldSimulation::HEIGHT));
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

    AgentPreset agent = vm.agent();
    ImGui::Text("Sensor Angle");
    if (ImGui::SliderFloat("##sensor_angle", &agent.sensor_angle, 0.0f, 2.0f)) { 
        vm.setAgent(agent);
    }

    ImGui::Text("Sensor Distance");
    if (ImGui::SliderFloat("##sensor_dist", &agent.sensor_dist, 1.0f, 12.0f)) {
        vm.setAgent(agent);
    }

    ImGui::Text("Turn Angle");
    if (ImGui::SliderFloat("##turn_angle", &agent.turn_angle, 0.0f, 1.0f)) {
        vm.setAgent(agent);
    }

    ImGui::Text("Step Size");
    if (ImGui::SliderFloat("##step_size", &agent.step_size, 0.1f, 5.0f)) {
        vm.setAgent(agent);
    }

    ImGui::Text("Evaporation");
    if (ImGui::SliderFloat("##evaporate", &agent.evaporate, 0.5f, 0.99f)) {
        vm.setAgent(agent);
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset")) {
        vm.reset();
    }
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Color Palette");
    auto palette = vm.palette();
    if (ImGui::ColorEdit3("Start", &palette[0].r, ImGuiColorEditFlags_NoLabel)) {
        vm.setPalette(palette);
    }
    if (ImGui::ColorEdit3("Midpoint", &palette[1].r, ImGuiColorEditFlags_NoLabel)) {
        vm.setPalette(palette);
    }
    if (ImGui::ColorEdit3("Endpoint", &palette[2].r, ImGuiColorEditFlags_NoLabel)) {
        vm.setPalette(palette);
    }
    ImGui::Text("Palette Midpoint");
    if (ImGui::SliderFloat("##palette_mid", &agent.palette_mid, 0.0f, 1.0f)) {
        vm.setAgent(agent);
    }
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
    if (ImGui::BeginCombo("##Behavior", vm.selectedPresetName().c_str())) {
        for (int i = 0; i < vm.presetsCount(); i++) {
            bool is_selected = (vm.selectedPreset() == i);
            if (ImGui::Selectable(vm.presetName(i).c_str(), is_selected)) {
                vm.selectAgentPreset(i);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("##Palette", vm.selectedPaletteName().c_str())) {
        for (int i = 0; i < vm.palettesCount(); i++) {
            bool is_selected = (vm.selectedPalette() == i);
            if (ImGui::Selectable(vm.paletteName(i).c_str(), is_selected)) {
                vm.selectPalettePreset(i);
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
    SDL_SetRenderDrawColor(m_p->renderer, 40, 40, 40, 255);
    SDL_RenderClear(m_p->renderer);

    // Define the destination rectangle for the main simulation area
    const SDL_FRect mainRect = { 0, 0, SlimeMoldSimulation::WIDTH, SlimeMoldSimulation::HEIGHT };

    // Upload pixel data and render simulation
    SDL_UpdateTexture(m_p->texture, nullptr, m_p->pixels.data(), SlimeMoldSimulation::WIDTH * 4);
    SDL_RenderTexture(m_p->renderer, m_p->texture, nullptr, &mainRect);

    // Render ImGui on top
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_p->renderer);
    SDL_RenderPresent(m_p->renderer);

    // Handle quit
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT)
            m_p->done = true;
    }

}