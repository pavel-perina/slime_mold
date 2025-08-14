// gradient_demo.cpp
#include <iostream>
#include <print>
#include <format>
#include <string>
#include <span>
#include <cmath>

// Your headers
#include "common/presets.h"
#include "common/colors.h"


// Convert RGB to ANSI 24-bit color escape code
std::string rgb_to_ansi(const ColorRGB& rgb) {
    // Clamp and scale to 0-255
    int r = static_cast<int>(std::clamp(rgb.r, 0.0f, 1.0f) * 255);
    int g = static_cast<int>(std::clamp(rgb.g, 0.0f, 1.0f) * 255);
    int b = static_cast<int>(std::clamp(rgb.b, 0.0f, 1.0f) * 255);
    return std::format("\x1b[38;2;{};{};{}m", r, g, b);
}

// Reset color
const std::string reset = "\x1b[0m";
const std::string block = "█";  // Full block character

// Print gradient bar
void print_gradient_bar(std::string_view name, std::span<const ColorRGB> colors) {
    std::print("{}", name);
    for (const auto& color : colors) {
        std::print("{}{}", rgb_to_ansi(color), block);
    }
    std::print("{}\n", reset);
}



int main() {

    const std::array<const char*, 3> cmapLabels = { "RGB", "LAB", "LCH" };
    const std::array<GradientFunction, 3> cmapFuncs = { 
	    RGBGradient, LABGradient, LCHGradient
    };
    constexpr std::size_t steps_per_segment = 32;  // 32 steps between each color
    constexpr std::size_t total_steps = 64;

    const auto& palettes = presetPalettes();

    for (const auto& preset : palettes) {
        std::println("{}:", preset.name);

        const auto& c0 = preset.palette[0];
        const auto& c1 = preset.palette[1];
        const auto& c2 = preset.palette[2];
	for (int i = 0; i < 3; ++i) {
		std::print("{} ", cmapLabels[i]);
		auto cmap1 = cmapFuncs[i](c0, c1, 32);
		for (int i = 0; i < 32; ++i)
			std::print("{}{}", rgb_to_ansi(cmap1[i]), block);
		auto cmap2 = cmapFuncs[i](c1, c2, 32);
		for (int i = 0; i < 32; ++i)
			std::print("{}{}", rgb_to_ansi(cmap2[i]), block);
		std::println("{}", reset);
		
	}
    }

    return 0;
}

