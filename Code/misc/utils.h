#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double INFTY = std::numeric_limits<double>::infinity();
const double PI = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * PI / 180.0;
}

inline double random_double() {
    // Returns a random real in [0, 1)
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min, max)
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    // Returns a random integer in [min, max]
    return static_cast<int>(random_double(min, max+1));
}

inline float random_float() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);

    return distribution(generator);
}

// Halton AA
float halton(int index, int base) {
    float result = 0.0;
    float f = 1.0 / base;
    int i = index;

    while (i > 0) {
        result += f * (i % base);
        i = static_cast<int>(floor(i / static_cast<float>(base)));
        f /= base;
    }

    return result;
}

// Common Headers

#include "../math/interval.h"
#include "../core/Ray.h"
#include "../math/vec3.h"
#include "../math/vec2.h"

#endif
