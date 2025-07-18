#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include <thread>
#include <chrono>

class camera {
    public:
        double aspect_ratio      = 1.0;  // Ratio of image with over height
        int    image_width       = 100;  // Render image width in pixel count
        int    samples_per_pixel = 10;   // Count of random samples for each pixel
        int    max_depth         = 10;   // Maximum number of ray bounces into scene
        color  background;               // Scene background color;

        double vfov = 90;                    // Vertical field of view
        point3 lookfrom = point3(0, 0, 0);   // Point the camera is looking from
        point3 lookat   = point3(0, 0, -1);  // Point the camera is looking at
        vec3   vup      = vec3(0, 1, 0);     // Camera-relative "up" direction

        double defocus_angle = 0;   // Variation angle of rays through each pixel
        double focus_dist    = 10;  // Distance from the camera lookfrom point to the plane of perfect focus

        int number_of_threads = 1;

        void render_process(const hittable& world, std::vector<color>& image) {

            // Hacky way ensure that each thread has a different seed.
            // This is effectively a more precise version of the classic time(NULL) seed typically used
            // to seed srand() in C.
            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto seed = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
            srand(seed);

            for (int j = 0; j < image_height; j++) {
                /*log progress*/
                std::clog << "\rScanlines remaining: " << (image_height - j) << " " << std::flush; 
                
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);

                    for (int sample = 0; sample < samples_per_thread; sample++) {
                        ray r = get_ray(i, j);
                        //pixel_color += ray_color(r, max_depth, world);
                        image[i + j*image_width] += ray_color(r, max_depth, world);
                    }
                    //write_color(std::cout, pixel_samples_scale * pixel_color);
                }
            }
        }

        void render(const hittable& world) {
            initialize();

            std::vector<std::thread> threads;
            std::vector<std::vector<color>> images;
            images.resize(number_of_threads);

            for (int i = 0; i < images.size(); i++) {
                images[i].resize(image_width * image_height);
                threads.emplace_back(
                    &camera::render_process, this, std::cref(world), std::ref(images[i])
                );
            }

            for (auto& t : threads) {
                t.join();
            }

            // average the pixel values and write to PPM file
            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255 \n"; // PPM Header
            for (int p_idx = 0; p_idx < image_width * image_height; p_idx++) {
 
                color pixel(0,0,0);

                for (const auto& img : images) {
                    pixel += img[p_idx];
                }

                write_color(std::cout, pixel_samples_scale * pixel);
            }
            std::clog << "\rDone.                     \n";
        }

    private:
        std::vector<color> image;
        int samples_per_thread;

        int    image_height;        // Render image height in pixel count
        double pixel_samples_scale; // Color scale factor for a sum of pixel samples
        point3 center;              // Camera center
        point3 pixel00_loc;         // Location of pixel 0,0
        vec3   pixel_delta_u;       // Offest to pixel to the right
        vec3   pixel_delta_v;       // Offest to pixel below
        vec3   u, v, w;             // Camera frame bassis vectors
        vec3   defocus_disk_u;      // Defocus disk horizontal radius
        vec3   defocus_disk_v;      // Defocus disk vertical radius

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height; //clamp to height of 1 pixel

            image.resize(image_width * image_height);

            // Will divide the given sample per pixel to the closest integer multiple 
            // of the number of threads
            samples_per_thread = int(samples_per_pixel / number_of_threads);

            pixel_samples_scale = 1.0 / samples_per_pixel;

            center = lookfrom;

            /*Determine viewport dimensions*/
            auto theta = degrees_to_radians(vfov);
            auto h = std::tan(theta/2);
            auto viewport_height = 2 * h * focus_dist;
            /*Note: not using aspect_ratio to calulate because that is the ideal ratio, 
                    not necessarily the actual ratio. Happens because image dimension are integers */
            auto viewport_width = viewport_height * (double(image_width)/image_height);

            /*Calculate the u,v,w unit basis vectors for the camera coordinate frame*/
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w,u);

            /*Calculate the vectors across the horizontal and down the vertical viewport edges*/
            auto viewport_u = viewport_width * u;    // --> left to right
            auto viewport_v = viewport_height * -v;  //  V  top to bottom

            /*Calculate the horizontal and vertial delta vectors from pixel to pixel*/
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            /*Calculate the the location of the upper left pixel*/
            auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2; 
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            /*Calculate the camera defocus disk basis vectors*/
            auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        ray get_ray(int i, int j) const {
            /*Construct a camera ray originating from the defocus disk and directed at the randomly
              sampled point around the pixel location i, j.*/
            
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc
                            + ((i + offset.x()) * pixel_delta_u)
                            + ((j + offset.y()) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;
            auto  ray_time = random_double();

            return ray(ray_origin, ray_direction, ray_time);
        }

        vec3 sample_square() const {
            /*Returns the vector to a random points in the [-.5, -.5] to [.5, .5] unit square*/
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

        point3 defocus_disk_sample() const {
            //Returns a random point in the camera defocus disk.
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0) {
                return color(0,0,0);
            }

            hit_record rec;

            if (!world.hit(r, interval(0.001, infinity), rec)) 
                return background;

            ray scattered;
            color attenuation;
            color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

            if (!rec.mat->scatter(r, rec, attenuation, scattered))
                return color_from_emission;


            color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

            return color_from_emission + color_from_scatter;
        }
};

#endif