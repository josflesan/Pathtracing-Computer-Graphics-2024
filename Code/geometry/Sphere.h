#ifndef SPHERE_H
#define SPHERE_H

#include "../core/Hittable.h"
#include "../math/vec3.h"
#include "../misc/utils.h"

class Sphere : public Hittable {
    public:

        Sphere(point3 _center, double _radius, shared_ptr<Material> _material, double _rotationAngle = 0) 
            : center(_center), radius(_radius), mat(_material), rotationAngle(_rotationAngle)
        {
            auto rvec = vec3(radius, radius, radius);
            bbox = aabb(center - rvec, center + rvec);    
        }

        aabb bounding_box() const override { return bbox; }

        bool intersect(const Ray& r, interval ray_t, HitRecord& rec) const override {
            vec3 oc = r.origin() - center;
            double a = dot(r.direction(), r.direction());
            double b = dot(oc, r.direction());
            double c = dot(oc, oc) - radius*radius;
            double discriminant = b*b - a*c;

            if (discriminant < 0) return false;
            double root = sqrt(discriminant);

            // Check the two possible solutions for t
            auto temp = (-b - root) / a;
            if (!ray_t.surrounds(temp)) {
                temp = (-b + root) / a;
                if (!ray_t.surrounds(temp))
                    return false;
            }

            // Record hit information
            rec.t = temp;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat = mat;

            // Calculate texture coordinates if necessary
            if (mat->isTextured())
                get_sphere_uv(outward_normal, rec.texture_u, rec.texture_v);

            return true;
        }

    private:
        point3 center;
        double radius;
        shared_ptr<Material> mat;
        double rotationAngle;
        aabb bbox;

        static void get_sphere_uv(const point3& p, double& u, double& v) {
            double theta = acos(-p.y());
            double phi = atan2(-p.z(), p.x()) + PI;

            u = phi / (2 * PI);
            v = theta / PI;
        }
};

#endif
