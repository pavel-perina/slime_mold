//! \file presets.h

#pragma once

#include "colors.h"

#include <array>
#include <string_view>
#include <vector>


struct PalettePreset {
    std::string_view name;
    std::array<color::Rgb, 3> palette;
};


struct AgentPreset {
    std::string_view name;
    float sensor_angle, sensor_dist, turn_angle, step_size, evaporate;
    float palette_mid;
};


extern const std::vector<AgentPreset>&   presetAgents();
extern const std::vector<PalettePreset>& presetPalettes();
