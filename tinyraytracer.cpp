#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
#include "objects.h"
#define M_PI 3.1416
const int width    = 1024;
const int height   = 768;
const float fov    = M_PI/2;
Material      ivory(Vec3f(0.4, 0.4, 0.3));
Material red_rubber(Vec3f(0.3, 0.1, 0.1));
Material backgroud(Vec3f(0.2, 0.4, 0.6));

bool intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit_p, Vec3f &normal, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i<spheres.size(); i++) {
        float dist_i;
        //击中更近的球体
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit_p = orig + dir*dist_i;
            normal = (hit_p - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
    return spheres_dist<1000;
}
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres) {
    Vec3f hit_p, normal;
    Material material;
    if (!intersect(orig, dir, spheres, hit_p, normal, material)) {
        return backgroud.color;
    }
    return material.color;
}
void renderpix(std::vector<Vec3f>&framebuffer) {
    std::vector<Sphere>spheres={Sphere(Vec3f(1, 1, -10), 1, ivory), Sphere(Vec3f(5, 5, -10), 2, red_rubber)};
    for (int j = 0; j<height; j++) {
        for (int i = 0; i<width; i++) {
            float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
            float y = -(2*(j + 0.5)/(float)height - 1)*tan(fov/2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir, spheres);
        }
    }
}

void render() {
    std::vector<Vec3f> framebuffer(width*height);
    renderpix(framebuffer);
    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
    render();

    return 0;
}

