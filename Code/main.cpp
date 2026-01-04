#include "misc/utils.h"

#include "core/Scene.h"
#include "misc/JsonParser.h"
#include "materials/Texture.h"

int main() {
    // const int numFrames = 150;

    // Load initial scene
    JsonParser sceneParser("video.json");
    Scene scene = sceneParser.parse();

    auto camera = scene.getCamera();
    auto world = scene.getWorld();
    auto lights = scene.getLights();

    // Main animation loop
    // for (int frame = 1; frame <= numFrames; ++frame) {
    //     // Update primitives in the scene as necessary
    //     camera->lookfrom += vec3(0.013, 0, 0);
    //     camera->lookat += vec3(0.013, 0, 0);
    //     lights[0]->setPosition(lights[0]->getPosition() - vec3(0.013, 0, 0));

    //     // Output the frame to a file
    //     std::string filename = "output/frame" + std::to_string(frame) + ".ppm";
    //     std::ofstream file(filename);
    //     camera->renderToPPM(world, lights, file);
    //     file.close();
    // }

    camera->render(world, lights);
}
