#include "ui_imgui/ui.h"
#include "common/slime_mold.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <string>
#include <array>

namespace {

enum CMapInterpolation
{
    CMAP_INTERP_RGB,
    CMAP_INTERP_LAB,
    CMAP_INTERP_LCH,
};

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////////////////////////
class Ui::Private {
public:
    //! Constructor
    Private();

    // SDL stuff
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
    bool done = false;
    
    AgentPreset agent;

    //! FPS counter
    uint64_t      last_counter = 0;

    //! Simulation
    SlimeMoldSimulation sim;

    //! SDL Texture pixels
    std::vector<uint8_t> pixels;

    //! Agent behaviour presets
    std::vector<AgentPreset> presets;

    //! Palette presets
    std::vector<PalettePreset> palettes;


    static constexpr size_t PALETTE_SIZE = 1024;
    int cmapInterpolation = CMAP_INTERP_LCH;
    int selectedPreset = 0;
    int selectedPalette = 0;
    static const std::array<const char*, 3> cmapLabels;

    ColorRGB paletteA = { 0.31f, 0.14f, 0.33f };
    ColorRGB paletteB = { 0.87f, 0.85f, 0.65f };
    ColorRGB paletteC = { 0.54f, 0.99f, 0.77f };
    //float palette_mid = 0.5f;

    std::vector<uint8_t> preparePalette();

    void renderToPixels(std::vector<uint8_t>& pixels, const float* field);

    std::string presetName(size_t index) const
    {
        return presets[index].name;
    }

    std::string selectedPresetName() const
    {
        return presetName(selectedPreset);
    }

    std::string paletteName(size_t index) const
    {
        return palettes[index].name;
    }

    std::string selectedPaletteName() const
    {
        return paletteName(selectedPalette);
    }

    void updateAgent(size_t index)
    {
        agent = presets[index];
    }

    void updatePalette(size_t index)
    {
        paletteA = palettes[index].paletteA;
        paletteB = palettes[index].paletteB;
        paletteC = palettes[index].paletteC;
    }

};


const std::array<const char*, 3> Ui::Private::cmapLabels = { "RGB", "LAB", "LCH" };


Ui::Private::Private()
{
    pixels.resize(SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT * 4, 0);
    presets  = presetAgents();
    palettes = presetPalettes();
    updateAgent(selectedPreset);
}


std::vector<uint8_t> Ui::Private::preparePalette()
{
    size_t mid = (size_t)(agent.palette_mid * PALETTE_SIZE);
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


void Ui::Private::renderToPixels(std::vector<uint8_t>& pixels, const float* field)
{
    const auto palette = preparePalette();
#if USE_AVX
    // Initialize scale and clamp
    const __m256 kVec = _mm256_set1_ps(10.0f * PALETTE_SIZE / 256.0f);
    const __m256 maxIdx = _mm256_set1_ps(static_cast<float>(PALETTE_SIZE - 1));

    // Process 8 pixels at a time
    constexpr size_t avxWidth = 8;
    for (size_t i = 0; i < SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT; i += avxWidth) {
        // Load 8 field values
        __m256 fieldVals = _mm256_loadu_ps(field + i);
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
#else
    constexpr float k = 10.0f * PALETTE_SIZE / 256.0f;
    const uint32_t* paletteU32 = reinterpret_cast<const uint32_t*>(palette.data());
    uint32_t* pixelsU32 = reinterpret_cast<uint32_t*>(pixels.data());
    for (int i = 0; i < SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT; i++) {
        int c = std::min(field[i] * k, static_cast<float>(PALETTE_SIZE - 1));
        //uint8_t c = (uint8_t)std::min(log(field[i]+2.73f)*20.f, 255.0f);
        pixelsU32[i] = paletteU32[c];
    }
#endif
}




Ui::Ui()
    : m_p(std::make_unique<Private>())
{
    constexpr size_t TOTAL_WIDTH = 224 + SlimeMoldSimulation::WIDTH;
    SDL_Init(SDL_INIT_VIDEO);
    m_p->window = SDL_CreateWindow("Slime Mold", TOTAL_WIDTH, SlimeMoldSimulation::HEIGHT, 0);
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

bool Ui::done()
{
    return m_p->done;
}


void Ui::frame() 
{
    // Do simulation step
    m_p->sim.step(m_p->agent);
    m_p->renderToPixels(m_p->pixels, m_p->sim.data());

    // Prepare a new frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    uint64_t current_counter = SDL_GetPerformanceCounter();
    float delta_time = (current_counter - m_p->last_counter) / (float)SDL_GetPerformanceFrequency();
    float fps = 1.0f / delta_time;
    m_p->last_counter = current_counter;

    AgentPreset& agent = m_p->agent;

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
    ImGui::Text("Sensor Angle");
    ImGui::SliderFloat("##sensor_angle", &agent.sensor_angle, 0.0f, 2.0f);
    ImGui::Text("Sensor Distance");
    ImGui::SliderFloat("##sensor_dist", &agent.sensor_dist, 1.0f, 12.0f);
    ImGui::Text("Turn Angle");
    ImGui::SliderFloat("##turn_angle", &agent.turn_angle, 0.0f, 1.0f);
    ImGui::Text("Step Size");
    ImGui::SliderFloat("##step_size", &agent.step_size, 0.1f, 5.0f);
    ImGui::Text("Evaporation");
    ImGui::SliderFloat("##evaporate", &agent.evaporate, 0.5f, 0.99f);
    ImGui::Spacing();
    if (ImGui::Button("Reset")) {
        m_p->sim.reset();
    }
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Color Palette");
    ImGui::ColorEdit3("Start",    &m_p->paletteA.r, ImGuiColorEditFlags_NoLabel);
    ImGui::ColorEdit3("Midpoint", &m_p->paletteB.r, ImGuiColorEditFlags_NoLabel);
    ImGui::ColorEdit3("Endpoint", &m_p->paletteC.r, ImGuiColorEditFlags_NoLabel);
    ImGui::Text("Palette Midpoint");
    ImGui::SliderFloat("##palette_mid", &agent.palette_mid, 0.0f, 1.0f);
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
    if (ImGui::BeginCombo("##Behavior", m_p->selectedPresetName().c_str())) {
        for (int i = 0; i < m_p->presets.size(); i++) {
            bool is_selected = (m_p->selectedPreset == i);
            if (ImGui::Selectable(m_p->presetName(i).c_str(), is_selected)) {
                m_p->selectedPreset = i; // FIXME: single line
                m_p->updateAgent(i);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("##Palette", m_p->selectedPaletteName().c_str())) {
        for (int i = 0; i < m_p->palettes.size(); i++) {
            bool is_selected = (m_p->selectedPalette == i);
            if (ImGui::Selectable(m_p->paletteName(i).c_str(), is_selected)) {
                m_p->selectedPalette = i;
                m_p->updatePalette(i);
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