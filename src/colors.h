#pragma once
#include <vector>

struct ColorRGB { float r, g, b; };
struct ColorLAB { float L, a, b; };
struct ColorLCH { float L, C, H; };

ColorLAB LCHtoLAB(const ColorLCH& lch);
ColorLCH LABtoLCH(const ColorLAB& lab);

ColorRGB LABtoRGB(const ColorLAB& lab);
ColorLAB RGBtoLAB(const ColorRGB& rgb);

std::vector<ColorRGB> LCHGradient2(const ColorRGB& start, const ColorRGB& end, std::size_t length);
