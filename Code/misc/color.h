#ifndef COLOR_H
#define COLOR_H

#include "../math/vec3.h"

#include <iostream>

using color = vec3;

inline double linear_to_gamma(double linear_component) {
    return pow(linear_component, 1.0 / 2.0);
}

inline color reinhardToneMapping(const color& pixel_color, double exposure) {
    double r = pixel_color.x() * exposure;
    double g = pixel_color.y() * exposure;
    double b = pixel_color.z() * exposure;
    
    // Calculate luminance of HDR color
    float luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;

    // Scale colour by luminance
    double tone_mapped_r = r / (1.0 + luminance);
    double tone_mapped_g = g / (1.0 + luminance);
    double tone_mapped_b = b / (1.0 + luminance);

    return color(tone_mapped_r, tone_mapped_g, tone_mapped_b);
}

inline color exponentialToneMapping(const color& pixel_color, double exposure) {
    double r = pixel_color.x();
    double g = pixel_color.y();
    double b = pixel_color.z();

    // Apply exponential tone mapping
    double tone_mapped_r = 1.0 - exp(-r * exposure);
    double tone_mapped_g = 1.0 - exp(-g * exposure);
    double tone_mapped_b = 1.0 - exp(-b * exposure);

    return color(tone_mapped_r, tone_mapped_g, tone_mapped_b);
}

void write_color(std::ostream &out, color pixel_color, int samples_per_pixel, double exposure) {
    // Divide the color by the number of samples
    auto scale = 1.0 / samples_per_pixel;
    auto scaled_color = pixel_color * scale;

    // Apply Reinhard tone mapping
    auto tone_mapped = reinhardToneMapping(scaled_color, exposure);
    auto r = tone_mapped.x();
    auto g = tone_mapped.y();
    auto b = tone_mapped.z();

    // Apply the linear to gamma transform
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Write translated [0, 255] value of each color component
    static const interval intensity(0.000, 0.9999);
    out << static_cast<int>(256 * intensity.clamp(r)) << ' '
        << static_cast<int>(256 * intensity.clamp(g)) << ' '
        << static_cast<int>(256 * intensity.clamp(b)) << '\n';
}

#endif
