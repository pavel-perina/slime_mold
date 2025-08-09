//! \file slime_mold_viewmodel.h
#pragma once
#include <string>
#include <memory>
#include <array>
#include "common/presets.h"

class SlimeMoldViewModel {
public:
    enum CMapInterpolation {
        CMAP_INTERP_RGB,
        CMAP_INTERP_LAB,
        CMAP_INTERP_LCH,
        CMAP_INTERP_END
    };

    SlimeMoldViewModel();

    ~SlimeMoldViewModel();

    // UI State
    void selectAgentPreset(size_t index);
    void selectPalettePreset(size_t index);
    size_t presetsCount() const;
    size_t palettesCount() const;
    size_t selectedPreset() const;
    size_t selectedPalette() const;
    std::string presetName(size_t index) const;
    std::string selectedPresetName() const;
    std::string paletteName(size_t index) const;
    std::string selectedPaletteName() const;

    AgentPreset agent() const;
    void setAgent(const AgentPreset&);

    std::array<ColorRGB, 3> palette() const;
    void setPalette(const std::array<ColorRGB, 3>&);


    void updatePixels(uint8_t* pixels);
    void reset();

private:
    class Private;
    std::unique_ptr<Private> m_p;
 };
