#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "Light.h"
#include "../core/Hittable.h"
#include "../misc/utils.h"

class AreaLight : public Light {
    public:
        AreaLight(const point3& corner, const vec3& edge1, const vec3& edge2, const color& intensity, const int numSamples)
            : corner(corner), edge1(edge1), edge2(edge2), intensity(intensity), numSamples(numSamples) {
                // Calculate the normal of the light source (assuming edges are perpendicular)
                normal = unit_vector(cross(edge1, edge2));
        }

        // Set the position of the light
        void setPosition(vec3 position) override {
            edge1 = position;
        }

        // Get the intensity of the light source
        vec3 getIntensity() const override {
            return intensity;
        }

        // Get the position of the light source
        vec3 getPosition() const override {
            return corner;
        }

        // Sample a random point on the light source
        color sampleLight(const HitRecord& rec, const Hittable& world) const override {
            color totalIllumination = color(0, 0, 0);

            for (int i = 0; i < numSamples; ++i) {
                // Sample point from surface of area light randomly
                double u = random_float();
                double v = random_float();

                vec3 sampledPoint = corner + u * edge1 + v * edge2;

                // Calculate the direction towards the light
                vec3 toLight = unit_vector(sampledPoint - rec.p);

                // Check if the point is in shadow
                Ray shadowRay(rec.p, toLight);
                HitRecord shadowRec;
                bool inShadow = world.intersect(shadowRay, interval(0.001, INFTY), shadowRec);

                // Accumulate illumination if not in shadow
                if (!inShadow) {
                    float cosTheta = dot(rec.normal, toLight);
                    float distance = (sampledPoint - rec.p).length() * (sampledPoint - rec.p).length();
                    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);

                    totalIllumination += attenuation * cosTheta * intensity * 2;
                }
            }

            // Average the accumulated illumination
            totalIllumination /= numSamples;

            return totalIllumination;
        }

    private:
        point3 corner;  // Corner of the rectangle
        vec3 edge1;     // First edge of the rectangle
        vec3 edge2;    // Second edge of the rectangle
        color intensity; // Intensity of the light source
        int numSamples; // Number of samples to take
        vec3 normal;    // Normal of the rectangle
};

#endif // AREALIGHT_H
