//! \file main.cpp

#include <algorithm>
#include <print>
#include <string>

#include "common/presets.h"
#include "common/colors.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Convert RGB to ANSI 24-bit color escape code
static std::string rgbToEscapeCode(const color::Rgb& rgb)
{
    // Clamp and scale to 0-255
    int r = static_cast<int>(std::clamp(rgb.r, 0.0f, 1.0f) * 255);
    int g = static_cast<int>(std::clamp(rgb.g, 0.0f, 1.0f) * 255);
    int b = static_cast<int>(std::clamp(rgb.b, 0.0f, 1.0f) * 255);
    return std::format("\x1b[38;2;{};{};{}m", r, g, b);
}


int main()
{

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const std::array<std::pair<std::string, color::GradientFunction>, 5> gradFuncs ({
        { "   RGB", color::gradientRgb    },
        { "CieLAB", color::gradientCieLab },
        { " OkLAB", color::gradientOkLab  },
        { "CieLCH", color::gradientCieLch },
        { " OkLCH", color::gradientOkLch  }
    });
    const auto &palettes = presetPalettes();

    for (const auto& preset : palettes) {
        std::println("\x1b[37;1m{}:\x1b[0m", preset.name);
        const auto& c0 = preset.palette[0];
        const auto& c1 = preset.palette[1];
        const auto& c2 = preset.palette[2];
        for (const auto& [gradLabel, gradFn] : gradFuncs) {
            std::print("\x1b[37;0m{}  ", gradLabel);
            auto cmap1 = gradFn(c0, c1, 32);
            for (int i = 0; i < 32; ++i) {
                std::print("{}\xE2\x96\x88", rgbToEscapeCode(cmap1[i]));
            }
            auto cmap2 = gradFn(c1, c2, 32);
            for (int i = 1; i < 32; ++i) {
                std::print("{}\xE2\x96\x88", rgbToEscapeCode(cmap2[i]));
            }
            std::println("\x1b[0m"); // reset color
        }
    }

    std::vector<std::tuple<std::string, color::Rgb, color::Rgb>> gradients ({
        { "Blue to white",  { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
        { "Blue to black",  { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
        { "Red to green",   { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { "Red to blue",    { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { "Blue to green",  { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { "Red to white",   { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
        { "Purple to gold", { 0.5f, 0.0f, 0.5f }, { 1.0f, .84f, 0.0f } },
    });
    for (const auto& [gradLabel, color1, color2] : gradients) {
        std::println("\x1b[37;1m{}:\x1b[0m", gradLabel);
        for (const auto& [gradFnLabel, gradFn] : gradFuncs) {
            std::print("{} ", gradFnLabel);
            auto cmap = gradFn(color1, color2, 32);
            for (int i = 0; i < 32; ++i) {
                std::print("{}\xE2\x96\x88", rgbToEscapeCode(cmap[i]));
            }
            std::println("\x1b[0m"); // reset color
        }
    }

    return 0;
}
