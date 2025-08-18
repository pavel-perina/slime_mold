//! \file main.cpp

#include <algorithm>
#include <array>
#include <print>
#include <string>

#include "common/presets.h"
#include "common/colors.h"

#ifdef _WIN32
#include <windows.h>
#endif


// Convert RGB to ANSI 24-bit color escape code
static std::string escRgb(const color::Rgb& rgb)
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

    // gradient steps (preferably even to make result symmetric)
    constexpr size_t gradientSteps = 55;
    constexpr std::string_view blockChar = "\xE2\x96\x88";
    constexpr std::string_view escWhite  = "\x1b[37;1m";
    constexpr std::string_view escGray   = "\x1b[37;0m";
    constexpr std::string_view escReset  = "\x1b[0m";

    const auto &palettes = presetPalettes();
    constexpr auto gradFuncs = std::to_array<std::pair<std::string_view, color::GradientFunction>> ({
        { "   RGB", color::gradientRgb    },
        { "CieLAB", color::gradientCieLab },
        { " OkLAB", color::gradientOkLab  },
        { "CieLCH", color::gradientCieLch },
        { " OkLCH", color::gradientOkLch  }
    });

    // === Render gradients for each preset palette ===
    for (const auto& preset : palettes) {
        std::println("{}{}:{}", escWhite, preset.name, escReset);
        for (const auto& [gradLabel, gradFn] : gradFuncs) {
            std::print("{}{}  ", escGray, gradLabel);
	    constexpr size_t gradientHalfLength = (gradientSteps+1)/2;
            auto cmap1 = gradFn(preset.palette[0], preset.palette[1], gradientHalfLength);
            for (int i = 0; i < gradientHalfLength; ++i) {
                std::print("{}{}", escRgb(cmap1[i]), blockChar);
            }
            auto cmap2 = gradFn(preset.palette[1], preset.palette[2], gradientHalfLength);
	    // do not repeat color at midpoint
            for (int i = 1; i < gradientHalfLength; ++i) {
                std::print("{}{}", escRgb(cmap2[i]), blockChar);
            }
            std::println(escReset);
        }
    }

    // === Render gradients for custom two-color test cases ===
    constexpr auto gradients = std::to_array<std::tuple<std::string_view, color::Rgb, color::Rgb>> ({
        { "Blue to white",  { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } },
        { "Blue to black",  { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
        { "Red to green",   { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { "Red to blue",    { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { "Blue to green",  { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { "Red to white",   { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
        { "Purple to gold", { 0.5f, 0.0f, 0.5f }, { 1.0f, .84f, 0.0f } },
    });

    for (const auto& [gradLabel, color1, color2] : gradients) {
        std::println("{}{}:{}", escWhite, gradLabel, escReset);
        for (const auto& [gradFnLabel, gradFn] : gradFuncs) {
            std::print("{}{}  ", escGray, gradFnLabel);
            auto cmap = gradFn(color1, color2, gradientSteps);
            for (int i = 0; i < gradientSteps; ++i) {
                std::print("{}{}", escRgb(cmap[i]), blockChar);
            }
            std::println(escReset);
        }
    }

    return 0;
}

