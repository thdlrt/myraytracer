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
Material      ivory(Vec2f(0.6, 0.3), Vec3f(0.4, 0.4, 0.3), 50.);
Material red_rubber(Vec2f(0.9, 0.1),Vec3f(0.3, 0.1, 0.1), 10.);
Material backgroud(Vec2f(0, 0), Vec3f(0.2, 0.7, 0.8), 0);
Material black(Vec2f(0, 0), Vec3f(0, 0, 0), 0);
std::vector<Sphere> spheres = {
    Sphere(Vec3f(-3, 0, -16), 2, ivory),
    Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber),
    Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber),
    Sphere(Vec3f(7, 5, -18), 4, ivory)
};
std::vector<PointLight> lights = {
    PointLight(Vec3f(-20, 20, 20), 1.5),
    PointLight(Vec3f(30, 50, -25), 1.8),
    PointLight(Vec3f(30, 20, 30), 1.7)
};

bool intersect(const Vec3f &orig, const Vec3f &dir, Vec3f &hit_p, Vec3f &normal, Material &material) {
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
Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir) {
    Vec3f hit_p, normal;
    Material material;
    if (!intersect(orig, dir, hit_p, normal, material)) {
        return backgroud.color;
    }
    float diffuse_intensity = 0, specular_intensity = 0;
    for(PointLight light: lights) {
        //阴影：判断是否被遮挡
        float dis = (light.position - hit_p).norm();
        Vec3f light_dir = (light.position - hit_p).normalize();
        //偏移原点，避免自己遮挡自己
        Vec3f shadow_orig = hit_p + light_dir*1e-3;
        Vec3f shadow_p, shadow_n;
        Material tmpmaterial;
        if (intersect(shadow_orig, light_dir, shadow_p, shadow_n, tmpmaterial) && (shadow_p-shadow_orig).norm() < dis) {
            continue;
        }
        //漫反射与镜面反射
        diffuse_intensity += light.intensity * std::max(0.f, light_dir*normal);
        //Vec3f mid = (light_dir - dir).normalize();
        //specular_intensity += light.intensity * powf(std::max(0.f, mid*normal), material.specular_exponent);
        specular_intensity += powf(std::max(0.f, -reflect(-light_dir, normal)*dir), material.specular_exponent)*light.intensity;
    }
    return material.color*diffuse_intensity*material.albedo[0] + Vec3f(1., 1., 1.)*specular_intensity*material.albedo[1];
}
void renderpix(std::vector<Vec3f>&framebuffer) {
    for (int j = 0; j<height; j++) {
        for (int i = 0; i<width; i++) {
            float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
            float y = -(2*(j + 0.5)/(float)height - 1)*tan(fov/2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir);
        }
    }
}

void render() {
    std::vector<Vec3f> framebuffer(width*height);
    renderpix(framebuffer);
    std::ofstream ofs;
    ofs.open("./out.ppm", std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < height*width; ++i) {
        for (int j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
    render();

    return 0;
}

