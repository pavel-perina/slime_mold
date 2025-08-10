#include "common/colors.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

constexpr float TO_RADIANS = std::numbers::pi_v<float> / 180.0f;
constexpr float TO_DEGREES = 180.0f / std::numbers::pi_v<float>;

inline float clamp(float x)
{
    return std::max(0.0f, std::min(1.0f, x));
}


// Gamma decode sRGB → Linear RGB
inline float invGamma(float c)
{
    return (c <= 0.04045f)
        ? (c / 12.92f)
        : std::pow((c + 0.055f) / 1.055f, 2.4f);
}


// Gamma encode Linear RGB → sRGB
inline float gammaCorrect(float c)
{
    return (c <= 0.0031308f)
        ? (12.92f * c)
        : (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f);
}


// Circular Hue interpolation (shortest arc)
inline float interpolateHue(float h1, float h2, float t)
{
    float delta = std::fmod(h2 - h1 + 540.0f, 360.0f) - 180.0f;
    return std::fmod(h1 + delta * t + 360.0f, 360.0f);
}

} // anonymous namespace


ColorLAB LCHtoLAB(const ColorLCH& lch)
{
    const float h_rad = lch.H * TO_RADIANS;
    return { lch.L, lch.C * std::cos(h_rad), lch.C * std::sin(h_rad) };
}


ColorLCH LABtoLCH(const ColorLAB& lab)
{
    float C = std::sqrt(lab.a * lab.a + lab.b * lab.b);
    float H = std::atan2(lab.b, lab.a) * TO_DEGREES;
    if (H < 0.0f)
        H += 360.0f;
    return { lab.L, C, H };
}


ColorLAB RGBtoLAB(const ColorRGB& rgb)
{
    const float r = invGamma(rgb.r);
    const float g = invGamma(rgb.g);
    const float b = invGamma(rgb.b);

    // Convert to XYZ (sRGB D65)
    float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
    float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
    float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;

    // Normalize by D65 white point
    x /= 0.95047f;
    y /= 1.00000f;
    z /= 1.08883f;

    // f(t) helper
    auto f = [](float t) -> float {
        return (t > 0.008856f) ? std::cbrt(t) : (7.787f * t + 16.0f / 116.0f);
        };

    const float fx = f(x);
    const float fy = f(y);
    const float fz = f(z);

    return {
        .L = (116.0f * fy) - 16.0f,
        .a = 500.0f * (fx - fy),
        .b = 200.0f * (fy - fz)
    };
}


ColorRGB LABtoRGB(const ColorLAB& lab)
{
    // D65 white point
    constexpr float refX = 95.047f;
    constexpr float refY = 100.000f;
    constexpr float refZ = 108.883f;

    float y = (lab.L + 16.0f) / 116.0f;
    float x = lab.a / 500.0f + y;
    float z = y - lab.b / 200.0f;

    x = (x * x * x > 0.008856f) ? (x * x * x) : ((x - 16.0f / 116.0f) / 7.787f);
    y = (y * y * y > 0.008856f) ? (y * y * y) : ((y - 16.0f / 116.0f) / 7.787f);
    z = (z * z * z > 0.008856f) ? (z * z * z) : ((z - 16.0f / 116.0f) / 7.787f);

    x *= refX;
    y *= refY;
    z *= refZ;

    // XYZ to linear RGB
    const float r = x *  0.032406f + y * -0.015372f + z * -0.004986f;
    const float g = x * -0.009689f + y *  0.018758f + z *  0.000415f;
    const float b = x *  0.000557f + y * -0.002040f + z *  0.010570f;

    return { .r = clamp(gammaCorrect(r)),  .g = clamp(gammaCorrect(g)), .b = clamp(gammaCorrect(b)) };
}


std::vector<ColorRGB> LCHGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length)
{
    std::vector<ColorRGB> gradient(length);

    ColorLCH startLCH = LABtoLCH(RGBtoLAB(startRGB));
    ColorLCH endLCH   = LABtoLCH(RGBtoLAB(endRGB));

    for (size_t i = 0; i < length; ++i) {
        float t = (float)i / (length - 1);

        ColorLCH lch;
        lch.L = (1.0f - t) * startLCH.L + t * endLCH.L;
        lch.C = (1.0f - t) * startLCH.C + t * endLCH.C;

        float deltaH = endLCH.H - startLCH.H;
        if (deltaH > 180.0f)
            deltaH -= 360.0f;
        if (deltaH < -180.0f)
            deltaH += 360.0f;
        lch.H = startLCH.H + t * deltaH;

        if (lch.H < 0.0f)
            lch.H += 360.0f;
        if (lch.H >= 360.0f)
            lch.H -= 360.0f;

        gradient[i] = LABtoRGB(LCHtoLAB(lch));
    }

    return gradient;
}


std::vector<ColorRGB> LABGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length)
{
    std::vector<ColorRGB> gradient(length);

    ColorLAB startLAB = RGBtoLAB(startRGB);
    ColorLAB endLAB   = RGBtoLAB(endRGB);

    for (size_t i = 0; i < length; ++i) {
        float t = (float)i / (length - 1);

        ColorLAB lab {
            .L = (1.0f - t) * startLAB.L + t * endLAB.L,
            .a = (1.0f - t) * startLAB.a + t * endLAB.a,
            .b = (1.0f - t) * startLAB.b + t * endLAB.b
        };

        gradient[i] = LABtoRGB(lab);
    }

    return gradient;
}


std::vector<ColorRGB> RGBGradient(const ColorRGB& startRGB, const ColorRGB& endRGB, std::size_t length)
{
    std::vector<ColorRGB> gradient(length);

    for (size_t i = 0; i < length; ++i) {
        float t = (float)i / (length - 1);
        gradient[i].r = (1.0f - t) * startRGB.r + t * endRGB.r;
        gradient[i].g = (1.0f - t) * startRGB.g + t * endRGB.g;
        gradient[i].b = (1.0f - t) * startRGB.b + t * endRGB.b;
    }

    return gradient;
}
