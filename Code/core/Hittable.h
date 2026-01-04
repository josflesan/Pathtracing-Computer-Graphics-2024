#ifndef HITTABLE_H
#define HITTABLE_H

#include "../core/Ray.h"
#include "../geometry/aabb.h"
#include "../misc/utils.h"

class Material;

class HitRecord {
    public:
        point3 p;
        vec3 normal;
        shared_ptr<Material> mat;
        double t;
        bool front_face;
        double texture_u;
        double texture_v;

        void set_face_normal(const Ray& r, const vec3& outward_normal) {
            // Sets the hit record normal vector
            // NOTE: the parameter `outward_normal` is assumed to have unit length

            front_face = dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }

        // Helper method to assign UV mapped coordinates to hit
        void set_uv(double u, double v) {
            texture_u = u;
            texture_v = v;
        }
};

class Hittable {
    public:
        // Default destructor
        virtual ~Hittable() = default;

        // Intersection method
        virtual bool intersect(const Ray& r, interval ray_t, HitRecord& rec) const = 0;

        virtual aabb bounding_box() const = 0;
};

#endif
