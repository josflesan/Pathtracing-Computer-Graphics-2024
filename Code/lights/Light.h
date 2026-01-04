#ifndef LIGHT_H
#define LIGHT_H

#include "../core/Scene.h"
#include "../math/vec3.h"

using std::string;

class Light {
    public:
        virtual color sampleLight(const HitRecord& rec, const Hittable& world) const = 0;
        virtual void setPosition(const vec3 position) = 0;
        virtual vec3 getPosition() const = 0;
        virtual vec3 getIntensity() const = 0;
        virtual ~Light() = default;
};

#endif
