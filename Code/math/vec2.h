#ifndef VEC2_H
#define VEC2_H

#include <cmath>
#include <iostream>

class vec2 {
    public:
        float x, y;

        // Constructors
        vec2() : x(0.0f), y(0.0f) {}
        vec2(float _x, float _y) : x(_x), y(_y) {}

        // Vector operations
        vec2 operator+(const vec2& other) const {
            return vec2(x + other.x, y + other.y);
        }

        vec2 operator-(const vec2& other) const {
            return vec2(x - other.x, y - other.y);
        }

        vec2 operator*(const float t) const {
            return vec2(x * t, y * t);
        }

        float dot(const vec2& other) const {
            return x * other.x + y * other.y;
        }

        float length() const {
            return sqrt(x * x + y * y);
        }

        vec2 normalize() const {
            float len = length();
            if (len != 0.0f) {
                return (*this) * (1.0f / len);
            } else {
                // Handle zero-length vector
                return vec2();
            }
        }

        // Print vector components
        friend std::ostream& operator<<(std::ostream& os, const vec2& v) {
            os << "(" << v.x << ", " << v.y << ")";
            return os;
        }
};

#endif  // VEC2_H
