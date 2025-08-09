//! \file slime_mold_viewmodel.cpp
#include "common/slime_mold_viewmodel.h"
#include "common/slime_mold_simulation.h"


class SlimeMoldViewModel::Private
{
public:
    Private();

    //! Simulation
    SlimeMoldSimulation sim;

    //! Agent behaviour presets
    std::vector<AgentPreset> presets;

    //! Palette presets
    std::vector<PalettePreset> palettes;

    size_t cmapInterpolation = CMAP_INTERP_LCH;
    size_t selectedPreset = 0;
    size_t selectedPalette = 0;
    static const std::array<const char*, 3> cmapLabels;

    static constexpr size_t PALETTE_SIZE = 1024;
    std::array<ColorRGB, 3> palette = { {
        { 0.31f, 0.14f, 0.33f },
        { 0.87f, 0.85f, 0.65f },
        { 0.54f, 0.99f, 0.77f },
    } };

    std::vector<uint8_t> preparePalette();

    void renderToPixels(std::vector<uint8_t>& pixels, const float* field);

    //! SDL Texture pixels
    std::vector<uint8_t> pixels;

    AgentPreset agent;

    //! FPS counter
    uint64_t last_counter = 0;

    /*static const std::array<const char*, 3> cmapLabels = { "RGB", "LAB", "LCH" };*/

};


SlimeMoldViewModel::Private::Private()
{
    pixels.resize(SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT * 4, 0);
    presets = presetAgents();
    palettes = presetPalettes();
}


std::vector<uint8_t> SlimeMoldViewModel::Private::preparePalette()
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
    auto g1 = gradientFn(palette[0], palette[1], mid);
    auto g2 = gradientFn(palette[1], palette[2], PALETTE_SIZE - mid);
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


SlimeMoldViewModel::SlimeMoldViewModel()
    : m_p(std::make_unique<Private>())
{
    selectAgentPreset(m_p->selectedPreset);
    selectPalettePreset(m_p->selectedPalette);
}


SlimeMoldViewModel::~SlimeMoldViewModel() = default;


void SlimeMoldViewModel::selectAgentPreset(size_t index)
{
    m_p->agent = m_p->presets[index];
}


void SlimeMoldViewModel::selectPalettePreset(size_t index)
{
    m_p->palette[0] = m_p->palettes[index].palette[0];
    m_p->palette[1] = m_p->palettes[index].palette[1];
    m_p->palette[2] = m_p->palettes[index].palette[2];
}

size_t SlimeMoldViewModel::presetsCount() const
{
    return m_p->presets.size();
}


size_t SlimeMoldViewModel::palettesCount() const
{
    return m_p->palettes.size();
}


size_t SlimeMoldViewModel::selectedPreset() const
{
    return m_p->selectedPreset;
}

size_t SlimeMoldViewModel::selectedPalette() const
{
    return m_p->selectedPalette;
}


std::string SlimeMoldViewModel::presetName(size_t index) const
{
    return m_p->presets[index].name;
}


std::string SlimeMoldViewModel::selectedPresetName() const
{
    return presetName(m_p->selectedPreset);
}


std::string SlimeMoldViewModel::paletteName(size_t index) const
{
    return m_p->palettes[index].name;
}


std::string SlimeMoldViewModel::selectedPaletteName() const
{
    return paletteName(m_p->selectedPalette);
}


AgentPreset SlimeMoldViewModel::agent() const 
{
    return m_p->agent;
}


void SlimeMoldViewModel::setAgent(const AgentPreset& a)
{
    m_p->agent = a;
}


std::array<ColorRGB, 3> SlimeMoldViewModel::palette() const
{
    return m_p->palette;
}


void SlimeMoldViewModel::setPalette(const std::array<ColorRGB, 3>& pal)
{
    m_p->palette = pal;
}


void SlimeMoldViewModel::updatePixels(uint8_t* pixels)
{
    
    m_p->sim.step(m_p->agent);
    const float* field = m_p->sim.data();

    const auto palette = m_p->preparePalette();
#if defined(USE_AVX2)
    // Initialize scale and clamp
    const __m256 kVec = _mm256_set1_ps(10.0f * Private::PALETTE_SIZE / 256.0f);
    const __m256 maxIdx = _mm256_set1_ps(static_cast<float>(Private::PALETTE_SIZE - 1));

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
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(pixels + i * 4), colors);
    }
#else
    constexpr float k = 10.0f * Private::PALETTE_SIZE / 256.0f;
    const uint32_t* paletteU32 = reinterpret_cast<const uint32_t*>(palette.data());
    uint32_t* pixelsU32 = reinterpret_cast<uint32_t*>(pixels);
    for (int i = 0; i < SlimeMoldSimulation::WIDTH * SlimeMoldSimulation::HEIGHT; i++) {
        int c = std::min(field[i] * k, static_cast<float>(Private::PALETTE_SIZE - 1));
        //uint8_t c = (uint8_t)std::min(log(field[i]+2.73f)*20.f, 255.0f);
        pixelsU32[i] = paletteU32[c];
    }
#endif
}


void SlimeMoldViewModel::reset()
{
    m_p->sim.reset();
}
