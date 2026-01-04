#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"
#include "../core/Scene.h"
#include "../math/vec3.h"   

class PointLight : public Light {
    public:
        PointLight() = default;
        PointLight(vec3 position, vec3 intensity) : position(position), intensity(intensity) {}

        // Set the position of the light
        void setPosition(vec3 position) override {
            this->position = position;
        }

        // Get the position of the light
        vec3 getPosition() const override {
            return position;
        }

        // Get the intensity of the light
        vec3 getIntensity() const override {
            return intensity;
        }

        // Method to sample the light source
        color sampleLight(const HitRecord& rec, const Hittable& world) const override {
            vec3 lightDir = unit_vector(position - rec.p);
            Ray shadowRay(rec.p, lightDir);
            HitRecord shadowRec;
            if (world.intersect(shadowRay, interval(0.001, INFTY), shadowRec)) {
                // Check if intersection is before the light source
                return vec3(0, 0, 0);
            }

            // Calculate the attenuation
            double NdotL = std::max(0.0, dot(rec.normal, lightDir));
            float distance = (position - rec.p).length() * (position - rec.p).length();
            return NdotL * intensity * 2 / distance;
        }

    private:
        vec3 position;  // Position of the point light
        vec3 intensity; // Intensity of the point light
};

#endif  // POINTLIGHT_H
