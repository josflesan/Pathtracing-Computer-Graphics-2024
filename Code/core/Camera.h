#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>

#include "../misc/utils.h"

#include "../core/Hittable.h"
#include "../misc/color.h"
#include "../materials/Material.h"

using std::string;

class Camera {
    public:
        string type              = "pinhole"; // Type of camera used for rendering
        string render_mode       = "phong";  // Rendering mode used by the camera
        color  background        = color(0, 0, 0);  // Background colour
        int    nbounces          = 1;    // Number of bounces for indirect illumination
        double aspect_ratio      = 1.0;  // Ratio of image width over height
        int    image_width       = 100;  // Rendered image width in pixel count
        int    image_height      = 0;    // Rendered image height in pixel count
        int    samples_per_pixel = 20;   // Count of random samples for each pixel

        double vfov              = 90;                  // Vertical view angle (field of view)
        double exposure          = 0.1;                 // Camera exposure for tone mapping
        point3 lookfrom          = point3(0, 0, -1);    // Point camera is looking from
        point3 lookat            = point3(0, 0, 0);     // Point camera is looking at
        vec3   vup               = vec3(0, 1, 0);       // Camera-relative "up" direction

        double lens_radius       = 0;    // Radius of camera lens
        
        void render(const Hittable& world, const std::vector<shared_ptr<Light>>& lights) {
            initialize();
            std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

            for (int j = 0; j < image_height; ++j) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0,0,0);

                    // If camera type is 'binary', use binary_ray_color method
                    if (render_mode == "binary") {
                        Ray r = get_ray(i, j, 1);
                        pixel_color = binary(r, world);
                        write_color(std::cout, pixel_color, 1, 1);
                    } else if (render_mode == "phong") {
                        for (int sample = 0; sample < samples_per_pixel; ++sample) {
                            Ray r = get_ray(i, j, sample);
                            pixel_color += blinn_phong(r, world, lights, nbounces);
                        }
                        write_color(std::cout, pixel_color, samples_per_pixel, exposure);
                    } else if (render_mode == "pathtracer") {
                        // Otherwise, use pathtracer code
                        for (int sample = 0; sample < samples_per_pixel; ++sample) {
                            Ray r = get_ray(i, j, sample);
                            pixel_color += pathtrace(r, nbounces, world, lights);
                        }
                        write_color(std::cout, pixel_color, samples_per_pixel, exposure);
                    }
                }
            }

            std::clog << "\rDone.           \n";
        }

        void renderToPPM(const Hittable& world, const std::vector<shared_ptr<Light>>& lights, std::ostream& output) {
            initialize();
            output << "P3\n" << image_width << " " << image_height << "\n255\n";

            for (int j = 0; j < image_height; ++j) {
                std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0,0,0);

                    // If camera type is 'binary', use binary_ray_color method
                    if (render_mode == "binary") {
                        Ray r = get_ray(i, j, 1);
                        pixel_color = binary(r, world);
                        write_color(output, pixel_color, 1, 1);
                    } else if (render_mode == "phong") {
                        for (int sample = 0; sample < samples_per_pixel; ++sample) {
                            Ray r = get_ray(i, j, sample);
                            pixel_color += blinn_phong(r, world, lights, nbounces);
                        }
                        write_color(output, pixel_color, samples_per_pixel, exposure);
                    } else if (render_mode == "pathtracer") {
                        // Otherwise, use pathtracer code
                        for (int sample = 0; sample < samples_per_pixel; ++sample) {
                            Ray r = get_ray(i, j, sample);
                            pixel_color += pathtrace(r, nbounces, world, lights);
                        }
                        write_color(output, pixel_color, samples_per_pixel, exposure);
                    }
                }
            }

            std::clog << "\rDone.           \n";
        }

    private:
        point3  origin;         // Camera origin
        point3  pixel00_loc;    // Location of pixel 0, 0
        vec3    pixel_delta_u;  // Offset to pixel to the right
        vec3    pixel_delta_v;  // Offset to pixel below
        vec3    u, v, w;        // Camera frame basis vectors
        vec3    defocus_disk_u; // Defocus disk horizontal radius
        vec3    defocus_disk_v; // Defocus disk vertical radius

        void initialize() {

            // Only assign image_height by aspect ratio if not passed in as input
            if (image_height == 0) {
                image_height = static_cast<int>(image_width / aspect_ratio);
                image_height = (image_height < 1) ? 1 : image_height;
            }

            origin = lookfrom;

            // Determine viewport dimensions
            // auto focal_length = (lookfrom - lookat).length();
            double focal_length = 1.0;
            double theta = degrees_to_radians(vfov);
            double h = tan(theta / 2);
            double aspect_ratio = static_cast<double>(image_width) / image_height;
            double viewport_height = 2.0 * h;
            double viewport_width = aspect_ratio * viewport_height;

            // Calculate the basis vectors for camera coordinate frame
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            // Calculate the horizontal and vertical vectors across the viewport
            vec3 viewport_u = viewport_width * u;       // Vector across viewport horizontal edge
            vec3 viewport_v = viewport_height * -v;     // Vector down viewport vertical edge

            // Calculate the offset to the pixel to the right and below
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            // Calculate the location of the upper left pixel
            auto viewport_upper_left = origin - (focal_length * w) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            // Calculate camera defocus disk
            defocus_disk_u = lens_radius * u;
            defocus_disk_v = lens_radius * v;
        }

        Ray get_ray(int i, int j, int sampleIndex) const {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);

            // If pathtracer, apply defocus and antialiasing
            if (render_mode == "pathtracer") {
                // Defocus: Uniform sampling
                vec3 lensPoint = uniformSamplingDefocus();
                vec3 focalPoint = origin + (defocus_disk_u * lensPoint[0]) + (defocus_disk_v * lensPoint[1]);

                // AA: Jitter the ray within the pixel
                int jitterX, jitterY;
                int baseX = 2;
                int baseY = 3;

                // Generate 2D Halton sample for jittering
                halton2D(sampleIndex, jitterX, jitterY, baseX, baseY);

                // Jitter the ray within the pixel
                float jitterXNormalized = jitterX / static_cast<float>(image_width);
                float jitterYNormalized = jitterY / static_cast<float>(image_height);

                pixel_center += (jitterXNormalized * pixel_delta_u) + (jitterYNormalized * pixel_delta_v);

                auto ray_direction = pixel_center - focalPoint;

                return Ray(focalPoint, ray_direction);
            }

            auto ray_direction = pixel_center - origin;

            return Ray(origin, ray_direction);
        }

        void halton2D(int index, int& x, int& y, int baseX, int baseY) const {
            x = static_cast<int>(halton(index, baseX) * image_width);
            y = static_cast<int>(halton(index, baseY) * image_height);
        }

        vec3 uniformSamplingDefocus() const {
            float r = sqrt(random_float());
            float theta = 2.0 * PI * random_float();

            // Compute sampled point on lens
            return vec3(r * cos(theta), r * sin(theta), 0);
        }

        color binary(const Ray& r, const Hittable& world) {
            HitRecord rec;

            // If there is an intersection, output solid red colour
            if (world.intersect(r, interval(0.001, INFTY), rec)) {
                return color(1, 0, 0);
            }

            // Otherwise, return black
            return color(0, 0, 0);
        }

        color blinn_phong(const Ray& r, const Hittable& world, const std::vector<shared_ptr<Light>>& lights, int depth) {
            HitRecord rec;

            // If there is an intersection
            if (world.intersect(r, interval(0.001, INFTY), rec)) {
                return rec.mat->getShading(world, lights, r, background, rec, nbounces);
            }

            return background;
        }

        color pathtrace(const Ray& r, int depth, const Hittable& world, const std::vector<shared_ptr<Light>>& lights) const {
            HitRecord rec;

            // If we've exceeded the ray bounce limit, no more light is gathered
            if (depth <= 0)
                return color(0, 0, 0);

            // Address Shadow Acne by setting min bound as 0.001
            if (world.intersect(r, interval(0.001, INFTY), rec)) {
                color directLighting = calculateDirectLighting(world, rec, lights);

                Ray scattered;
                color attenuation;
                if (rec.mat->evaluate(r, rec, attenuation, scattered))
                    return attenuation * directLighting + attenuation * pathtrace(scattered, depth-1, world, lights);
                else
                    return directLighting;  // Surface is non-reflective, only consider direct lighting
            }

            return background;
        }

        color calculateDirectLighting(const Hittable& world, const HitRecord& rec, const std::vector<shared_ptr<Light>>& lights) const {
            color directLighting = color(0, 0, 0);

            for (const auto& light : lights) {
                directLighting += light->sampleLight(rec, world);
            }

            return directLighting;
        }
};

#endif
