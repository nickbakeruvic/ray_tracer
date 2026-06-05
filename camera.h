#ifndef CAMERA_H
#define CAMERA_H

#include <cuda_runtime.h>

#include "hittable_list.h"
#include "hittable.h"
#include "material.h"

__global__ void render_gpu(vec3 *image, size_t num_pixels, int image_height, int image_width, int samples_per_pixel, int max_depth, hittable_list world) {
    auto tid = blockIdx.x * blockDim.x + threadIdx.x;
    double row = tid % image_width;
    double col = tid / image_width;

    if (tid >= num_pixels)
        return;

    double a = row / double(image_height);
    double b = col / double(image_height);

    //printf("\nrow %d col %d tid %d, %f, %f", (int)row, (int)col, (int)tid, 255 * a, 255 * b);
    /*color pixel_color(0,0,0);
    for (int sample = 0; sample < samples_per_pixel; sample++) {
        ray r = get_ray(i, j);
        pixel_color += ray_color(r, max_depth, world);
    }
    image[j][i] = write_color(pixel_samples_scale * pixel_color);*/

    image[tid] = vec3(255 * a, 255 * b, 100);
}

class camera {
  public:
    double aspect_ratio = 1.0;
    int image_width  = 100;
    int samples_per_pixel = 10;
    int max_depth = 10;

    double vfov = 90;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat = point3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist = 10;

    void render(const hittable& world, std::ostream& out = std::cout) {
        initialize();

        size_t num_pixels = image_height * image_width;
        size_t image_buf_size = num_pixels * sizeof(vec3);

        size_t blocks = int(num_pixels / 1024) + 1;
        vec3 *device_image = nullptr;
        hittable_list device_world;
        vec3 *host_image = (vec3 *)std::malloc(image_buf_size);

        cudaMalloc((void **)&device_image, num_pixels * sizeof(vec3));
        cudaMalloc((void **) &device_world, sizeof(world));
        cudaMemcpy((void *)device_world, (void *)world, sizeof(world), cudaMemcpyHostToDevice);


        render_gpu<<<blocks, 1024>>>(device_image, num_pixels, image_height, image_width, samples_per_pixel, max_depth, device_world);

        cudaDeviceSynchronize();
        cudaMemcpy(host_image, device_image, image_buf_size, cudaMemcpyDeviceToHost);

        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        /*
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                image[j][i] = write_color(pixel_samples_scale * pixel_color);
            }
        }
        */

        for (int j = 0; j < image_height * image_width; j++) {
            out << host_image[j].x() << ' ' << host_image[j].y() << ' ' << host_image[j].z() << '\n';
        }

        std::clog << "\rDone.                   \n";
    }

  private:
    int image_height;   // Rendered image height
    double pixel_samples_scale; // Color scale factor for sum of pixel samples
    point3 center;  // Camera center
    point3 pixel00_loc; // Location of pixel 0, 0
    vec3 pixel_delta_u; // Offset to pixel to the right
    vec3 pixel_delta_v; // Offset to pixel below
    vec3 u, v, w; // Camera frame basis vectors
    vec3 defocus_disk_u;
    vec3 defocus_disk_v;

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left =
            center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // project camera ray towards randomly sampled point on pixel i,j
    RAY_HOST_DEVICE ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    RAY_HOST_DEVICE vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    RAY_HOST_DEVICE point3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    RAY_HOST_DEVICE color ray_color(const ray& r, int depth, const hittable& world) const {
        hit_record rec;

        if (depth <= 0)
            return color(0, 0, 0);

        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};

#endif