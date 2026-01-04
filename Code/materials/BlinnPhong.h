#ifndef BLINNPHONG_H
#define BLINNPHONG_H

#include "Material.h"
#include "Texture.h"
#include "../core/Scene.h"
#include "../math/vec3.h"
#include "../math/interval.h"
#include "../misc/utils.h"

class BlinnPhong : public Material {
    public:
        shared_ptr<Texture> texture;
        color diffuse_color;
        color specular_color;
        double specular_exponent;
        double ks;
        double kd;
        double reflectivity;
        double refractiveIndex;
        bool is_reflective;
        bool is_refractive;
        double transparency;
        
        // Solid color constructor
        BlinnPhong(const color& _diffColor, const color& _specColor, double _specExp, double _ks, double _kd, double _reflectivity, double _refractiveIndex, bool _isReflective, bool _isRefractive, double _transparency) 
            : diffuse_color(_diffColor), specular_color(_specColor), specular_exponent(_specExp), ks(_ks), kd(_kd), reflectivity(_reflectivity), refractiveIndex(_refractiveIndex), is_reflective(_isReflective), is_refractive(_isRefractive), transparency(_transparency) {}

        // Texture constructor
        BlinnPhong(shared_ptr<Texture>& _texture, const color& _diffColor, const color& _specColor, double _specExp, double _ks, double _kd, double _reflectivity, double _refractiveIndex, bool _isReflective, bool _isRefractive, double _transparency)
            : texture(_texture), diffuse_color(_diffColor), specular_color(_specColor), specular_exponent(_specExp), ks(_ks), kd(_kd), reflectivity(_reflectivity), refractiveIndex(_refractiveIndex), is_reflective(_isReflective), is_refractive(_isRefractive), transparency(_transparency) {}

        color getShading(const Hittable& world, const std::vector<shared_ptr<Light>>& lights, const Ray& r_in, const color& backgroundColor, HitRecord& rec, int depth) const override {
            // Determine if ray hits an object
            if (!world.intersect(r_in, interval(0.001, INFTY), rec)) {
                return backgroundColor;
            }

            vec3 view_direction = unit_vector(-r_in.direction());  // Direction from hit point to camera

            // Initialize diffuse and specular components
            color diffuse(0, 0, 0);
            color specular(0, 0, 0);

            // Iterate through all light sources            
            for (const auto& light : lights) {
                // Calculate the direction from hit point to light source
                vec3 light_direction = light->getPosition() - rec.p;
                float distance = light_direction.length() * light_direction.length();
                light_direction = unit_vector(light_direction);

                // Calculate halfway vector between view direction and light direction
                vec3 h = unit_vector(view_direction + light_direction);

                float lambertian = std::max(0.0, dot(rec.normal, light_direction));
                float specular_angle = std::max(0.0, dot(rec.normal, h));
                vec3 intensity = light->sampleLight(rec, world);

                // Accumulate the diffuse and specular contributions from current light source
                diffuse += lambertian * intensity * 2 / distance;
                specular += pow(specular_angle, specular_exponent) * specular_color * intensity * 2 / distance;
            }

            // If material is textured, multiply diffuse by texture_color
            if (rec.mat->isTextured()) {
                diffuse = diffuse * rec.mat->getTexture()->getTextureColor(rec.texture_u, rec.texture_v);
            } else {
                diffuse = diffuse * diffuse_color;
            }

            // Calculate final colour
            color shading = (kd * diffuse + ks * specular);

            // Add ambient term
            if (rec.mat->isTextured())
                shading += color(0.2, 0.2, 0.2) * rec.mat->getTexture()->getTextureColor(rec.texture_u, rec.texture_v);
            else
                shading += color(0.2, 0.2, 0.2) * diffuse_color;

            // Handle reflections
            if (rec.mat->isReflective() && depth > 0 && rec.mat->getReflectivity() > 0) {
                // Set the scattered ray to be the reflected ray
                vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
                Ray reflectedRay = Ray(rec.p, reflected);
                color reflection_color = backgroundColor;
                HitRecord reflectHit;
                
                // Calculate the reflection color using the blinn_phong function
                if (world.intersect(reflectedRay, interval(0.001, INFTY), reflectHit))
                    reflection_color = reflectHit.mat->getShading(world, lights, reflectedRay, backgroundColor, reflectHit, depth - 1);

                // Multiply the reflection color by the material's reflectivity
                shading = (reflection_color * reflectivity);
            }

            // Handle refractions
            if (rec.mat->isRefractive() && depth > 0) {
                double refractiveIndex = rec.mat->getRefractiveIndex();
                double refractiveIndexRatio = rec.front_face ? 1.0 / refractiveIndex : refractiveIndex;

                vec3 unit_direction = unit_vector(r_in.direction());
                double cos_theta = dot(-unit_direction, rec.normal);
                double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

                // Total internal reflection if above critical angle or Schlick approx
                bool cannot_refract = refractiveIndexRatio * sin_theta > 1.0;
                if (cannot_refract || schlick(cos_theta, refractiveIndexRatio) > random_double()) {
                    // Reflection
                    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
                    Ray reflectedRay = Ray(rec.p, reflected);
                    color reflection_color = backgroundColor;
                    HitRecord reflectHit;
                    
                    // Calculate the reflection color using the blinn_phong function
                    if (world.intersect(reflectedRay, interval(0.001, INFTY), reflectHit))
                        reflection_color = reflectHit.mat->getShading(world, lights, reflectedRay, backgroundColor, reflectHit, depth - 1);

                    double attenuation = exp(-rec.mat->getTransparency() * rec.t);
                    shading = (reflection_color * attenuation);
                } else {
                    // Refraction
                    vec3 refracted = refract(unit_direction, rec.normal, refractiveIndexRatio);
                    Ray refractedRay = Ray(rec.p, refracted);
                    color refraction_color = backgroundColor;
                    HitRecord refractHit;

                    // Calculate the refraction color
                    if (world.intersect(refractedRay, interval(0.001, INFTY), refractHit))
                        refraction_color = refractHit.mat->getShading(world, lights, refractedRay, backgroundColor, refractHit, depth-1);

                    // Use Beer's Law to attenuate the color based on distance
                    double attenuation = exp(-rec.mat->getTransparency() * rec.t);
                    shading = (refraction_color * attenuation);
                }
            }
            

            return shading;  // Reflection always occurs in Blinn-Phong model
        }

        bool isReflective() const {
            return is_reflective;
        }

        bool isRefractive() const {
            return is_refractive;
        }

        double getReflectivity() const {
            return reflectivity;
        }

        double getRefractiveIndex() const {
            return refractiveIndex;
        }

        double getTransparency() const {
            return transparency;
        }

        shared_ptr<Texture> getTexture()const  {
            return texture;
        }

        bool isTextured() const {
            return texture == nullptr ? false : true;
        }

    private:

        double schlick(double cosine, double ref_idx) const {
            double r0 = (1 - ref_idx) / (1 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1 - r0) * pow((1 - cosine), 5);
        }
};


#endif // BLINNPHONG_H
