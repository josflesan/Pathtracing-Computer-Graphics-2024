#ifndef CYLINDER_H
#define CYLINDER_H

#include "../core/Hittable.h"
#include "../math/vec3.h"
#include "../misc/utils.h"

class Cylinder : public Hittable {
    public:
        Cylinder(const point3& _center, vec3 _axis, double _radius, double _height, shared_ptr<Material> _material)
            : center(_center), axis(_axis), radius(_radius), height(_height), mat(_material) {
                vec3 minimumExtreme, maximumExtreme;

                if (axis.x() == 1) {
                    // If aligned with the x-axis...
                    minimumExtreme = vec3(center.x() - height, center.y() - radius, center.z() - radius);
                    maximumExtreme = vec3(center.x() + height, center.y() + radius, center.z() + radius);
                } else if (axis.y() == 1) {
                    // If aligned with the y-axis...
                    minimumExtreme = vec3(center.x() - radius, center.y() - height, center.z() - radius);
                    maximumExtreme = vec3(center.x() + radius, center.y() + height, center.z() + radius);
                } else if (axis.z() == 1) {
                    // If aligned with the z-axis...
                    minimumExtreme = vec3(center.x() - radius, center.y() - radius, center.z() - height);
                    maximumExtreme = vec3(center.x() + radius, center.y() + radius, center.z() + height);
                }

                bbox = aabb(minimumExtreme, maximumExtreme);
            }

        aabb bounding_box() const override { return bbox; }

        bool intersect(const Ray& r, interval ray_t, HitRecord& rec) const override {
            // Define parameters
            vec3 oc = r.origin() - center;

            double a = r.direction().length_squared() - dot(r.direction(), axis) * dot(r.direction(), axis);
            double b = 2.0 * (dot(oc, r.direction()) - dot(oc, axis) * dot(r.direction(), axis));
            double c = oc.length_squared() - dot(oc, axis) * dot(oc, axis) - radius * radius;
            double discriminant = b * b - 4 * a * c;

            // Check for intersection
            if (discriminant > 0) {
                double root1 = (-b - sqrt(discriminant)) / (2.0 * a);
                double root2 = (-b + sqrt(discriminant)) / (2.0 * a);

                // Check if roots are within valid range
                if (!ray_t.contains(root1) && !ray_t.contains(root2)) {
                    return false;
                }
                
                // Check for intersection within the height of the cylinder
                double hit_point = root1 < root2 ? root1 : root2;
                double projection = dot((r.origin() + hit_point * r.direction()) - center, axis);

                /* Check for intersections with bottom and top planes
                * Planes equations containing the c1 and c2 points with their d normals are
                * d . (x - ci) = 0
                * 
                * Ray's parametric equation is
                * x(t) = ray.direction() * t + ray.origin()
                * 
                * So we must solve
                * d.(ray.direction()*t + ray.origin() - ci) = 0
                * t = -(d.(ray.origin() - ci))/(d.ray.direction())
                */

               // If ray and axis are parallel, ray does not hit the caps
               if (dot(r.direction(), axis) != 0) {
                    vec3 c1 = center - height * axis;
                    vec3 c2 = center + height * axis;
                    double root3 = -(dot(axis, r.origin() - c1)) / (dot(axis, r.direction()));
                    double root4 = -(dot(axis, r.origin() - c2)) / (dot(axis, r.direction()));
                    vec3 point3 = r.at(root3);
                    vec3 point4 = r.at(root4);

                    // If hit_point is closer, do not render bottom cap (avoid superposition)
                    if (hit_point >= root3 && root3 > 0 && (point3 - c1).length_squared() <= radius * radius) {
                        // Record hit information (intersection with bottom cap)
                        rec.t = root3;
                        rec.p = point3;
                        vec3 normal = unit_vector(axis);
                        rec.set_face_normal(r, normal);
                        rec.mat = mat;
                        
                        return true;
                    }

                    if (root4 > 0 && (point4 - c2).length_squared() <= radius * radius) {
                        // Record hit information (intersection with top cap)
                        rec.t = root4;
                        rec.p = point4;
                        vec3 normal = unit_vector(axis);
                        rec.set_face_normal(r, normal);
                        rec.mat = mat;

                        return true;
                    }
               }

                if (projection < -height || projection > height) {
                    return false;
                }

                // Record hit information (intersection with main body)
                rec.t = hit_point;
                rec.p = r.at(rec.t);
                vec3 normal = unit_vector(rec.p - center - projection * axis);
                rec.set_face_normal(r, normal);
                rec.mat = mat;

                // Calculate texture coordinates if necessary
                if (mat->isTextured())
                    get_cylinder_uv(rec.p, rec.texture_u, rec.texture_v);

                return true;
            }

            return false;
        }

    private:
        point3 center;
        vec3 axis;
        double radius;
        double height;
        shared_ptr<Material> mat;
        aabb bbox;

        void get_cylinder_uv(const point3& p, double& u, double& v) const {
            // Calculate the azimuthal angle around the cylinder for any orientation
            double phi;

            if (axis.x() == 1) {
                phi = atan2(p.y(), p.z());
            } else if (axis.y() == 1) {
                phi = atan2(p.x(), p.z());
            } else if (axis.z() == 1) {
                phi = atan2(p.y(), p.x());
            }

            // Map the projection of the point onto the cylinder's height to the v texture coordinate
            v = dot(p - center, axis) / height;
            v = (v + 1) / 2;
            u = phi / (2 * PI);
        }
};

#endif
