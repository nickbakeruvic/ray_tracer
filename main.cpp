#include <fstream>
#include "utils.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"

color ray_color(const ray& r, const hittable& world) {
	hit_record rec;
	if (world.hit(r, interval(0, infinity), rec)) {
		return 0.5 * (rec.normal + color(1, 1, 1));
	}

	vec3 unit_direction = unit_vector(r.direction());
	auto a = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main() {
	/**************
	 * World
	 **************/
	hittable_list world;

	auto material_ground = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, material_ground));

	for (int i = -11; i < 11; i ++) {
		for (int j = -11; j < 11; j ++) {
			auto choose_mat = random_double();
			point3 center(i + 0.9 * random_double(), 0.2, j + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				} else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				} else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material_dielectric = make_shared<dielectric>(1.5);
	auto material_lambertian = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	auto material_metal = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);

	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material_dielectric));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material_lambertian));
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material_metal));

	/**************
	 * Camera
	 **************/
	camera cam;
	
	cam.aspect_ratio = 16.0 / 9.0;
	cam.image_width = 1200;
	cam.samples_per_pixel = 500;
	cam.max_depth = 50;

	cam.vfov = 20;
	cam.lookfrom = point3(13, 2, 3);
	cam.lookat = point3(0, 0, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0.6;
	cam.focus_dist = 10.0;

	std::ofstream out_file("image.ppm", std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out_file) {
		std::cerr << "Could not open image.ppm for writing\n";
		return 1;
	}

	cam.render(world, out_file);
}
