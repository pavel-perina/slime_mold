#pragma once
#include "colors.h"
#include <string>
#include <vector>

struct PalettePreset {
    std::string name;
    ColorRGB paletteA, paletteB, paletteC;
};

struct AgentPreset {
    std::string name;
    float sensor_angle, sensor_dist, turn_angle, step_size, evaporate;
    float palette_mid;
};

extern const std::vector<AgentPreset>   presetAgents();
extern const std::vector<PalettePreset> presetPalettes();
