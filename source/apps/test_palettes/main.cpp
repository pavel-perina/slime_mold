//! \file main.cpp

#include <algorithm>
#include <print>
#include <string>

#include "common/presets.h"
#include "common/colors.h"

// Convert RGB to ANSI 24-bit color escape code
static std::string rgbToEscapeCode(const ColorRGB& rgb)
{
    // Clamp and scale to 0-255
    int r = static_cast<int>(std::clamp(rgb.r, 0.0f, 1.0f) * 255);
    int g = static_cast<int>(std::clamp(rgb.g, 0.0f, 1.0f) * 255);
    int b = static_cast<int>(std::clamp(rgb.b, 0.0f, 1.0f) * 255);
    return std::format("\x1b[38;2;{};{};{}m", r, g, b);
}


int main()
{
    const std::array<const char*, 3> cmapLabels = { "RGB", "LAB", "LCH" };
    const std::array<GradientFunction, 3> cmapFuncs = { RGBGradient, LABGradient, LCHGradient };
    const auto& palettes = presetPalettes();
    for (const auto& preset : palettes) {
        std::println("{}:", preset.name);
        const auto& c0 = preset.palette[0];
        const auto& c1 = preset.palette[1];
        const auto& c2 = preset.palette[2];
        for (int i = 0; i < 3; ++i) {
            std::print("{} ", cmapLabels[i]);
            auto cmap1 = cmapFuncs[i](c0, c1, 32);
            for (int i = 0; i < 32; ++i) {
                std::print("{}█", rgbToEscapeCode(cmap1[i]));
            }
            auto cmap2 = cmapFuncs[i](c1, c2, 32);
            for (int i = 1; i < 32; ++i) {
                std::print("{}█", rgbToEscapeCode(cmap2[i]));
            }
            std::println("\x1b[0m"); // reset color
        }
    }
    return 0;
}
