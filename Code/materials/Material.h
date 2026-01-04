#ifndef MATERIAL_H
#define MATERIAL_H

#include "../misc/utils.h"
#include "../misc/color.h"

#include "../lights/Light.h"
#include "../materials/Texture.h"

class HitRecord;

class Material {
    public:

        virtual ~Material() = default;

        virtual bool isTextured() const = 0;
        virtual bool isReflective() const {
            return false;
        };
        virtual bool isRefractive() const {
            return false;
        };
        virtual color getReflectance(const HitRecord& rec) const {
            return color(0, 0, 0);
        };
        virtual double getRefractiveIndex() const {
            return 0;
        };
        virtual double getReflectivity() const {
            return 0;
        };
        virtual double getTransparency() const {
            return 0;
        }
        virtual shared_ptr<Texture> getTexture() const = 0;

        // Evaluate method for BRDF materials
        virtual bool evaluate(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const {
            return false;
        };

        // Shading method for Blinn-Phong materials
        virtual color getShading(
            const Hittable& world, const std::vector<shared_ptr<Light>>& lights, const Ray& r_in, const color& backgroundColor, HitRecord& rec, int depth) const {
                return color(0, 0, 0);
            };
};

#endif
