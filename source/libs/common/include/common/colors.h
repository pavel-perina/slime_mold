//! \file colors.h
//! \author Pavel Perina
//! \date July 2025

#pragma once

#include <vector>

struct ColorRGB { float r, g, b; };
struct ColorLAB { float L, a, b; };
struct ColorLCH { float L, C, H; };

extern ColorLAB LCHtoLAB(const ColorLCH& lch);
extern ColorLCH LABtoLCH(const ColorLAB& lab);

extern ColorRGB LABtoRGB(const ColorLAB& lab);
extern ColorLAB RGBtoLAB(const ColorRGB& rgb);

using GradientFunction = std::vector<ColorRGB>(*)(const ColorRGB&, const ColorRGB&, std::size_t);

extern std::vector<ColorRGB> LCHGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length);
extern std::vector<ColorRGB> LABGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length);
extern std::vector<ColorRGB> RGBGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length);
