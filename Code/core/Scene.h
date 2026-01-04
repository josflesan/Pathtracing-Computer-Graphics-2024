#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "HittableList.h"
#include "../geometry/bvh.h"
#include "../lights/Light.h"
#include "../misc/utils.h"

class Scene {
    public:
        Scene(shared_ptr<Camera> camera, HittableList& world, std::vector<shared_ptr<Light>>& lights) 
            : camera(camera), world(world), lights(lights) {}

        // Get the camera in the scene
        const shared_ptr<Camera>& getCamera() const {
            return camera;
        }

        // Get the list of lights in the scene
        const std::vector<shared_ptr<Light>> getLights() const {
            return lights;
        }

        // Get the world in the scene
        const HittableList& getWorld() const {
            return world;
        }

    private:
        shared_ptr<Camera> camera;
        HittableList world;
        std::vector<shared_ptr<Light>> lights;

};

#endif
