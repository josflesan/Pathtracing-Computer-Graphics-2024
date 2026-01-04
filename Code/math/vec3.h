#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3 {
    public:
        double e[3];

        // Constructors
        vec3(): e{0, 0, 0} {}
        vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

        // Accessors
        double x() const { return e[0]; }
        double y() const { return e[1]; }
        double z() const { return e[2]; }

        // Vector operations
        vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
        double operator[](int i) const { return e[i]; }
        double& operator[](int i) { return e[i]; }

        vec3& operator=(const vec3 &v) {
            e[0] = v.e[0];
            e[1] = v.e[1];
            e[2] = v.e[2];
            return *this;
        }

        vec3& operator+=(const vec3& v) {
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];

            return *this;
        }

        vec3& operator*=(const double t) {
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;

            return *this;
        }

        vec3& operator/=(const double t) {
            return *this *= 1/t;
        }

        // Component-wise division for Tone Mapping
        vec3& operator/(const vec3 &v) {
            e[0] /= v.e[0];
            e[1] /= v.e[1];
            e[2] /= v.e[2];
            return *this;
        }

        double length() const {
            return sqrt(length_squared());
        }

        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }

        // Make vector unit length
        void normalize() {
            *this /= length();
        }

        // Static methods for vector operations
        static double dot(const vec3& v1, const vec3& v2) {
            return v1.e[0]*v2.e[0]
                 + v1.e[1]*v2.e[1]
                 + v1.e[2]*v2.e[2];
        }

        static vec3 cross(const vec3& v1, const vec3& v2) {
            return vec3((v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1]),
                        (v1.e[2]*v2.e[0] - v1.e[0]*v2.e[2]),
                        (v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0]));
        }

        static vec3 random() {
            return vec3(random_double(), random_double(), random_double());
        }

        static vec3 random(double min, double max) {
            return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
        }
};

// Type aliases for vec3
using point3 = vec3;  // 3D point
using color = vec3;  // RGB color

// Utility functions for vec3
inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.x() << ' ' << v.y() << ' ' << v.z();
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.x() + v.x(), u.y() + v.y(), u.z() + v.z());
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.x() - v.x(), u.y() - v.y(), u.z() - v.z());
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
    return vec3(u.x() * v.x(), u.y() * v.y(), u.z() * v.z());
}

inline vec3 operator*(double t, const vec3 &v) {
    return vec3(t * v.x(), t * v.y(), t * v.z());
}

inline vec3 operator*(const vec3 &v, double t) {
    return t * v;
}

inline vec3 operator/(vec3 v, double t) {
    return (1/t) * v;
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline double dot(const vec3 &u, const vec3 &v) {
    return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.y()*v.z() - u.z()*v.y(), u.z()*v.x() - u.x()*v.z(), u.x()*v.y() - u.y()*v.x());
}

vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v, n)*n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

inline vec3 random_in_unit_sphere() {
    while (true) {
        auto p = vec3::random(-1, 1);
        if (p.length_squared() < 1)
            return p;
    }
}

inline vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0)  // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

#endif
