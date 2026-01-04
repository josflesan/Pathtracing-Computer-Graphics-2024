#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>
#include <algorithm>

#include "../core/Hittable.h"
#include "../math/vec3.h"

// Overall structure and barycentric coordinate calculation inspired by scratchapixel tutorial (https://scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates.html)
class Triangle : public Hittable {
    public:
        // Barycentric coordinates
        double alpha, beta, gamma;

        Triangle(vec3 _vertex1, vec3 _vertex2, vec3 _vertex3, shared_ptr<Material> _material) 
            : vertex1(_vertex1), vertex2(_vertex2), vertex3(_vertex3), mat(_material) {
                // Sort the vertices counter-clockwise
                std::vector<vec3> sortedVertices = sortCounterClockwise();
                vertex1 = sortedVertices[0];
                vertex2 = sortedVertices[1];
                vertex3 = sortedVertices[2];

                // Calculate the bounding box
                vec3 minimumExtreme = vec3(
                    std::min({vertex1.x(), vertex2.x(), vertex3.x()}),
                    std::min({vertex1.y(), vertex2.y(), vertex3.y()}),
                    std::min({vertex1.z(), vertex2.z(), vertex3.z()})
                );

                vec3 maximumExtreme = vec3(
                    std::max({vertex1.x(), vertex2.x(), vertex3.x()}),
                    std::max({vertex1.y(), vertex2.y(), vertex3.y()}),
                    std::max({vertex1.z(), vertex2.z(), vertex3.z()})
                );

                bbox = aabb(minimumExtreme, maximumExtreme);
            }

        aabb bounding_box() const override { return bbox; }

        bool intersect(const Ray& r, interval ray_t, HitRecord& rec) const override {
            // Calculate the normal of the triangle
            // This cross-product computes area of the paralellogram formed by the two edges
            vec3 normal = cross(vertex2 - vertex1, vertex3 - vertex1);
            float denom = dot(normal, normal);

            // Check if ray is parallel to the triangle (no intersection)
            float NdotRayDirection = dot(normal, r.direction());
            if (fabs(NdotRayDirection) < std::numeric_limits<double>::epsilon()) {
                return false;
            }

            // Calculate the distance from the ray origin to the triangle plane
            float d = -dot(normal, vertex1);
            double t = -(dot(r.origin(), normal) + d) / NdotRayDirection;

            // Check if the intersection point is within the valid range
            if (!ray_t.surrounds(t)) {
                return false;
            }

            // Calculate the intersection point
            point3 point_on_plane = r.at(t);

            // Check if point is inside the triangle
            vec3 edge1 = vertex2 - vertex1;
            vec3 edge2 = vertex3 - vertex2;
            vec3 edge3 = vertex1 - vertex3;

            // Barycentric coordinates
            double alpha, beta, gamma;

            vec3 C1 = cross(edge1, point_on_plane - vertex1);
            if (dot(normal, C1) < 0) return false;  // P is on the right side
            vec3 C2 = cross(edge2, point_on_plane - vertex2);
            alpha = dot(normal, C2);
            if (alpha < 0) return false;  // P is on the right side
            vec3 C3 = cross(edge3, point_on_plane - vertex3); 
            beta = dot(normal, C3);
            if (beta < 0) return false;  // P is on the right side

            if (dot(C1, normal) >= 0 && alpha >= 0 && beta >= 0) {
                // Record the hit information
                rec.t = t;
                rec.p = point_on_plane;
                rec.normal = -unit_vector(normal);
                rec.mat = mat;

                // Calculate texture coordinates if necessary
                if (mat->isTextured()) {
                    alpha /= denom;
                    beta /= denom;
                    gamma = 1 - alpha - beta;

                    // Find barycentric point on surface
                    vec2 uv1 = vec2(0, 0);
                    vec2 uv2 = vec2(0, 1);
                    vec2 uv3 = vec2(1, 1);
                    vec2 barycentric_point = uv1 * alpha + uv2 * beta + uv3 * gamma;

                    rec.texture_u = barycentric_point.x;
                    rec.texture_v = barycentric_point.y;
                }
                
                return true;
            }

            return false;
        }

    private:
        vec3 vertex1;
        vec3 vertex2;
        vec3 vertex3;
        shared_ptr<Material> mat;
        aabb bbox;

        std::vector<vec3> sortCounterClockwise() const {
            // Determine vertex-texture mappings
            std::vector<vec3> vertices = {vertex1, vertex2, vertex3};

            // Sort vertices according to polar angle with centroid
            vec3 centroid(0.0f, 0.0f, 0.0f);
            for (const auto& vertex : vertices) {
                centroid.e[0] += vertex.x();
                centroid.e[1] += vertex.y();
                centroid.e[2] += vertex.z();
            }
            centroid /= vertices.size();

            std::sort(vertices.begin(), vertices.end(), [centroid](const vec3& v1, const vec3& v2) {
                return std::atan2(v1.y() - centroid.y(), v1.x() - centroid.x()) < std::atan2(v2.y() - centroid.y(), v2.x() - centroid.x());
            });

            return vertices;
        }
};

#endif
