#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "../geometry/aabb.h"
#include "../core/Hittable.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class HittableList : public Hittable {
    public:
        std::vector<shared_ptr<Hittable>> objects;

        HittableList() {}
        HittableList(shared_ptr<Hittable> object) { add(object); }

        void clear() { objects.clear(); }

        void add(shared_ptr<Hittable> object) {
            objects.push_back(object);
            bbox = aabb(bbox, object->bounding_box());
        }

        bool intersect(const Ray& r, interval ray_t, HitRecord& rec) const override {
            HitRecord temp_rec;
            bool hit_anything = false;
            auto closest_so_far = ray_t.max;

            for (const auto& object : objects) {
                if (object->intersect(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        aabb bbox;
};

#endif
