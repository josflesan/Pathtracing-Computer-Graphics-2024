#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <json/json.h>

#include "../core/Camera.h"
#include "../core/HittableList.h"
#include "../core/Scene.h"
#include "../geometry/bvh.h"
#include "../geometry/Cylinder.h"
#include "../geometry/Sphere.h"
#include "../geometry/Triangle.h"
#include "../lights/PointLight.h"
#include "../lights/AreaLight.h"
#include "../materials/BlinnPhong.h"
#include "../materials/BRDF.h"

class JsonParser {
    public:
        JsonParser(const std::string& filename) : filename(filename) {}

        Scene parse() {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open JSON file");
            }

            Json::CharReaderBuilder reader;
            Json::Value root;
            std::string errs;
            Json::parseFromStream(reader, file, &root, &errs);

            file.close();

            // Parse render mode and camera settings
            shared_ptr<Camera> cam = parseCamera(root);

            return parseScene(root, cam);
        }
    
    private:
        static vec3 parseVector(const Json::Value& jsonVector) {
            return vec3(jsonVector[0].asDouble(), jsonVector[1].asDouble(), jsonVector[2].asDouble());
        }

        static vec3 parseVectorRotate(const Json::Value& jsonVector) {
            // Invert sign of z-axis to change from LHS to RHS
            return vec3(jsonVector[0].asDouble(), jsonVector[1].asDouble(), -jsonVector[2].asDouble());
        }

        static vec3 parsePoint(const Json::Value& jsonVector) {
            return point3(jsonVector[0].asDouble(), jsonVector[1].asDouble(), -jsonVector[2].asDouble());
        }

        static color parseColor(const Json::Value& jsonVector) {
            return color(jsonVector[0].asDouble(), jsonVector[1].asDouble(), jsonVector[2].asDouble());
        }

        // Parse the camera section of the JSON
        static shared_ptr<Camera> parseCamera(const Json::Value& root) {
            Camera cam;
            cam.type = root["camera"]["type"].asString();
            cam.render_mode = root["rendermode"].asString();
            cam.nbounces = root["nbounces"].asInt();
            cam.background = parseColor(root["scene"]["backgroundcolor"]);
            cam.image_width = root["camera"]["width"].asInt();
            cam.image_height = root["camera"]["height"].asInt();
            cam.lookfrom = parsePoint(root["camera"]["position"]);
            cam.lookat = parsePoint(root["camera"]["lookAt"]);
            cam.vup = parseVector(root["camera"]["upVector"]);
            cam.vfov = root["camera"]["fov"].asDouble();
            cam.exposure = root["camera"]["exposure"].asDouble();
            cam.lens_radius = root["camera"]["lensRadius"].asDouble();

            return make_shared<Camera>(cam);
        }

        // Parse the Scene section of the JSON
        static Scene parseScene(const Json::Value& root, shared_ptr<Camera> cam) {
            string renderMode = root["rendermode"].asString();

            // Parse scene settings
            HittableList objects;
            const Json::Value& shapesArray = root["scene"]["shapes"];
            for (const auto& shapeJson : shapesArray) {
                string type = shapeJson["type"].asString();
                if (type == "sphere") {
                    if (renderMode == "phong") {
                        auto sphere_material = parseBlinnPhongMaterial(shapeJson["material"]);
                        objects.add(make_shared<Sphere>(parseVectorRotate(shapeJson["center"]), shapeJson["radius"].asDouble(), sphere_material));
                    } else {
                        auto sphere_material = parseBRDFMaterial(shapeJson["material"]);
                        objects.add(make_shared<Sphere>(parseVectorRotate(shapeJson["center"]), shapeJson["radius"].asDouble(), sphere_material, 3));
                    }

                } else if (type == "cylinder") {
                    if (renderMode == "phong") {
                        auto cylinder_material = parseBlinnPhongMaterial(shapeJson["material"]);
                        objects.add(make_shared<Cylinder>(parseVectorRotate(shapeJson["center"]), parseVector(shapeJson["axis"]), shapeJson["radius"].asDouble(), shapeJson["height"].asDouble(), cylinder_material));
                    } else {
                        auto cylinder_material = parseBRDFMaterial(shapeJson["material"]);
                        objects.add(make_shared<Cylinder>(parseVectorRotate(shapeJson["center"]), parseVector(shapeJson["axis"]), shapeJson["radius"].asDouble(), shapeJson["height"].asDouble(), cylinder_material));
                    }
                } else if (type == "triangle") {
                    if (renderMode == "phong") {
                        auto triangle_material = parseBlinnPhongMaterial(shapeJson["material"]);
                        objects.add(make_shared<Triangle>(parseVectorRotate(shapeJson["v0"]), parseVectorRotate(shapeJson["v1"]), parseVectorRotate(shapeJson["v2"]), triangle_material));
                    } else {
                        auto triangle_material = parseBRDFMaterial(shapeJson["material"]);
                        objects.add(make_shared<Triangle>(parseVectorRotate(shapeJson["v0"]), parseVectorRotate(shapeJson["v1"]), parseVectorRotate(shapeJson["v2"]), triangle_material));
                    }
                }
            }
            // Turn to BVH tree
            objects = HittableList(make_shared<bvh_node>(objects));

            // Parse light settings
            std::vector<shared_ptr<Light>> lights = {};
            const Json::Value& lightsArray = root["scene"]["lightsources"];
            for (const auto& lightJson : lightsArray) {
                if (lightJson["type"].asString() == "pointlight") {
                    lights.push_back(make_shared<PointLight>(parsePoint(lightJson["position"]), parseVector(lightJson["intensity"])));
                } else {
                    lights.push_back(make_shared<AreaLight>(parsePoint(lightJson["corner"]), parseVector(lightJson["edge1"]), parseVector(lightJson["edge2"]), parseVector(lightJson["intensity"]), lightJson["samples"].asInt()));
                }
            }

            Scene scene(cam, objects, lights);
            return scene;
        }

        static shared_ptr<BlinnPhong> parseBlinnPhongMaterial(const Json::Value& jsonMaterial) {
            shared_ptr<Texture> texture;
            if (jsonMaterial["texture"]) {
                texture = make_shared<Texture>(jsonMaterial["texture"].asCString());
            }

            auto material = make_shared<BlinnPhong>(
                texture,
                parseColor(jsonMaterial["diffusecolor"]),
                parseColor(jsonMaterial["specularcolor"]),
                jsonMaterial["specularexponent"].asDouble(),
                jsonMaterial["ks"].asDouble(),
                jsonMaterial["kd"].asDouble(),
                jsonMaterial["reflectivity"].asDouble(),
                jsonMaterial["refractiveindex"].asDouble(),
                jsonMaterial["isreflective"].asBool(),
                jsonMaterial["isrefractive"].asBool(),
                jsonMaterial["transparency"].asDouble()
            );

            return material;
        }

        static shared_ptr<Material> parseBRDFMaterial(const Json::Value& jsonMaterial) {
            shared_ptr<Texture> texture;
            if (jsonMaterial["texture"]) {
                texture = make_shared<Texture>(jsonMaterial["texture"].asCString());
            }

            if (jsonMaterial["brdfType"] == "lambertian") {
                auto material = make_shared<Lambertian>(parseColor(jsonMaterial["diffusecolor"]), texture);
                return material;
            } else if (jsonMaterial["brdfType"] == "schlick") {
                // Schlick material (without refractions)
                auto material = make_shared<SchlickBRDF>(
                    jsonMaterial["reflectance"].asFloat()
                );
                return material;
            } else {
                // Schlick material (with refractions)
                auto material = make_shared<SchlickRefractionsBRDF>(
                    jsonMaterial["reflectance"].asFloat()
                );
                return material;
            }
        }

        std::string filename;
};

#endif
