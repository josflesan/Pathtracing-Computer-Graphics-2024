#ifndef BRDF_H
#define BRDF_H

#include <algorithm>

#include "../misc/utils.h"
#include "../misc/color.h"
#include "Material.h"

// Lambertian BRDF
class Lambertian : public Material {
    public:
        Lambertian(const vec3& albedo, shared_ptr<Texture>& _texture) : albedo(albedo), texture(_texture) {}

        bool evaluate(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const override {
            vec3 scatter_direction = rec.normal + random_unit_vector();
            scattered = Ray(rec.p, scatter_direction);

            if (texture != nullptr)
                attenuation = texture->getTextureColor(rec.texture_u, rec.texture_v);
            else
                attenuation = albedo;

            return true;
        }

        // Reflectance method
        color getReflectance(const HitRecord& rec) const override {
            if (texture != nullptr)
                return texture->getTextureColor(rec.texture_u, rec.texture_v);
            else
                return albedo;
        }
        shared_ptr<Texture> getTexture() const override { return texture; }
        bool isTextured() const override { return texture != nullptr; }

    private:
        color albedo = color(0, 0, 0);
        shared_ptr<Texture> texture;
};

// Cook-Torrance BRDF
// class CookTorrance : public Material {
//     public:
//         CookTorrance(const color& albedo, shared_ptr<Texture>& _texture, float roughness, float fresnelReflectance) : albedo(albedo), texture(_texture), roughness(roughness), fresnelReflectance(fresnelReflectance) {}

//         bool evaluate(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const override {
//             // Calculate the half-vector            
//             vec3 h = unit_vector(-r_in.direction() + vec3(0, 1, 0));

//             // Calculate the Beckmann distribution function
//             float alpha = roughness;
//             float alpha2 = alpha * alpha;
//             float NdotH = dot(rec.normal, h);
//             float beckmannExp = (NdotH * NdotH - 1.0) / (alpha2 * NdotH * NdotH);
//             float beckmannD = exp(-beckmannExp) / (PI * alpha2  * NdotH * NdotH * NdotH * NdotH);

//             // Calculate the geometric attenuation
//             double NdotL = std::max(0.0, dot(rec.normal, vec3(0, 1, 0)));
//             double NdotV = std::max(0.0, dot(rec.normal, -r_in.direction()));
//             double G = std::min(1.0, std::min(2.0 * NdotH * NdotV / NdotH, 2.0f * NdotH * NdotL / NdotL));

//             // Calculate the Fresnel reflection
//             float F = fresnelReflectance + (1.0f - fresnelReflectance) * pow(1.0 - NdotH, 5.0);

//             // Calculate the final BRDF value
//             float brdf = (beckmannD * G * F) / (4.0 * NdotL * NdotV);

//             // Calculate the attenuation colour
//             if (texture != nullptr)
//                 attenuation = brdf * texture->getTextureColor(rec.texture_u, rec.texture_v);
//             else
//                 attenuation = brdf * albedo;

//             // Update scattered ray
//             scattered = Ray(rec.p, vec3(0, 1, 0));

//             return true;
//         }

//         // Reflectance method
//         color getReflectance(const HitRecord& rec) const override {
//             if (texture != nullptr)
//                 return texture->getTextureColor(rec.texture_u, rec.texture_v);
//             else
//                 return albedo;
//         }
//         shared_ptr<Texture> getTexture() const override { return texture; }
//         bool isTextured() const override { return texture == nullptr; }

//     private:
//         color albedo;
//         shared_ptr<Texture> texture;
//         float roughness;
//         float fresnelReflectance;
// };

// Schlick BRDF (with refractions)
class SchlickRefractionsBRDF : public Material {
    public:
        float fresnelReflectance;

        SchlickRefractionsBRDF(float fresnelReflectance) : fresnelReflectance(fresnelReflectance) {}

        // Evaluate Schlick BRDF
        bool evaluate(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const override {
            // Calculate the reflection direction
            vec3 reflected = reflect(r_in.direction(), rec.normal);

            // Calculate the Fresnel reflection
            float cosTheta = dot(unit_vector(-r_in.direction()), rec.normal);
            float F = fresnelSchlick(cosTheta, fresnelReflectance);

            // Determine whether to reflect or refract based on Fresnel reflection
            if (random_float() < F) {
                // Reflect
                scattered = Ray(rec.p, reflected);
                attenuation = color(1.0, 1.0, 1.0);  // Reflectance color
            } else {
                // Refract
                double refractiveIndexRatio = rec.front_face ? 1.0 / 1.5 : 1.5;
                vec3 refracted = refract(r_in.direction(), rec.normal, refractiveIndexRatio);

                vec3 unit_direction = unit_vector(r_in.direction());
                double cos_theta = dot(-unit_direction, rec.normal);
                double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
                bool cannot_refract = refractiveIndexRatio * sin_theta > 1.0;
                if (!cannot_refract) {
                    scattered = Ray(rec.p, refracted);
                    attenuation = color(1, 1, 1);  // Refractive color

                    // Use Beer's Law to attenuate the color based on distance
                    double beersLaw = exp(-0.2 * rec.t);
                    attenuation *= beersLaw;
                } else {
                    // Total internal reflection
                    scattered = Ray(rec.p, reflected);
                    attenuation = color(1.0, 1.0, 1.0);  // Reflectance color

                    double beersLaw = exp(-0.2 * rec.t);
                    attenuation *= beersLaw;
                }
            }

            return true;
        }

        color getReflectance(const HitRecord& rec) const override {
            return color(1, 1, 1);
        }
        shared_ptr<Texture> getTexture() const override { return nullptr; }
        bool isTextured() const override { return false; }

    private:
        float fresnelSchlick(float cosTheta, float reflectance) const {
            return reflectance + (1.0 - reflectance) * pow(1.0 - cosTheta, 5.0);
        }
};

// Schlick BRDF (without refractions)
class SchlickBRDF : public Material {
    public:
        float fresnelReflectance;

        SchlickBRDF(float fresnelReflectance) : fresnelReflectance(fresnelReflectance) {}

        // Evaluate Schlick BRDF
        bool evaluate(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const override {
            // Calculate the reflection direction
            vec3 reflected = reflect(r_in.direction(), rec.normal);

            // Calculate the Fresnel reflection
            float cosTheta = dot(unit_vector(-r_in.direction()), rec.normal);
            float F = fresnelSchlick(cosTheta, fresnelReflectance);

            // Determine whether to reflect or refract based on Fresnel reflection
            if (random_float() < F) {
                // Reflect
                scattered = Ray(rec.p, reflected);
                attenuation = color(1.0, 1.0, 1.0);  // Reflectance color
            }

            return true;
        }

        color getReflectance(const HitRecord& rec) const override {
            return color(1, 1, 1);
        }
        shared_ptr<Texture> getTexture() const override { return nullptr; }
        bool isTextured() const override { return false; }

    private:
        float fresnelSchlick(float cosTheta, float reflectance) const {
            return reflectance + (1.0 - reflectance) * pow(1.0 - cosTheta, 5.0);
        }
};

#endif  // BRDF_H
