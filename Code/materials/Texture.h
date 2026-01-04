#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <fstream>
#include <vector>

#include "../misc/color.h"

struct Color {
    int r, g, b;
};

class Texture {
    public:
        Texture(const char* _filename) {
            loadPPM(_filename);
        }

        // Function to get texture color at given coordinates
        color getTextureColor(double u, double v) const {
            // Clamp input texture coordinates to [0, 1] x [1, 0]
            u = interval(0, 1).clamp(u);
            v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

            int x = static_cast<int>(u * (texture[0].size() - 1));
            int y = static_cast<int>(v * (texture.size() - 1));
            auto pixel = texture[y][x];

            auto color_scale = 1.0 / 255.0;
            return color(pixel.r * color_scale, pixel.g * color_scale, pixel.b * color_scale);
        }

        // Getter methods to obtain width and height of texture
        int getWidth() const {
            return static_cast<int>(texture[0].size());
        }

        int getHeight() const {
            return static_cast<int>(texture.size());
        }

    private:
        std::vector<std::vector<Color>> texture;

        // Function to load PPM texture
        bool loadPPM(const char* filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open the file: " << filename << std::endl;
                return false;
            }

            // Read in format
            std::string format;
            file >> format;
            if (format != "P6") {
                std::cerr << "Error: Invalid PPM file format. Expected P6." << std::endl;
                return false;
            }

            // Read in width, height and maxColor
            int width, height, maxColor;
            file >> width >> height >> maxColor;

            texture.resize(height, std::vector<Color>(width));

            // Consume newline character after maxColor
            file.get();

            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    // Read RGB values byte by byte
                    file.read(reinterpret_cast<char*>(&texture[i][j].r), 1);
                    file.read(reinterpret_cast<char*>(&texture[i][j].g), 1);
                    file.read(reinterpret_cast<char*>(&texture[i][j].b), 1);
                }
            }

            return true;
        }
};

#endif
