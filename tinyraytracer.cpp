#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
#include "objects.h"
//Whitted风格光线追踪
#define M_PI 3.1416
const int width    = 1024;
const int height   = 768;
const float fov    = M_PI/2;
const int max_depth = 4;

Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
Material backgroud(0.0, Vec4f(0, 0, 0, 0), Vec3f(0.2, 0.7, 0.8), 0);
Material black(0.0 , Vec4f(0, 0, 0,0), Vec3f(0, 0, 0), 0);

std::vector<Sphere> spheres = {
    Sphere(Vec3f(-5, 0, -32), 4, ivory),
    Sphere(Vec3f(-3, -1.5, -12), 2, glass),
    Sphere(Vec3f( 2, -0.5, -18), 3, red_rubber),
    Sphere(Vec3f( 7, 4, -18), 4, mirror)
};
std::vector<PointLight> lights = {
    PointLight(Vec3f(-20, 20, 20), 1.5),
    PointLight(Vec3f(30, 50, -25), 1.8),
    PointLight(Vec3f(30, 20, 30), 1.7)
};
Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) {
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    //假定光线从空气（折射率为1）进入另一种介质
    float etai = 1, etat = refractive_index;
    Vec3f n = N;
    if (cosi < 0) {//从介质进入空气
        cosi = -cosi;
        std::swap(etai, etat);
        n = -N;
    }
    float eta = etai/etat;
    float k = 1 - eta*eta*(1 - cosi*cosi);//是否全反射（cos<0）
    return k<0 ? Vec3f(0, 0, 0) : I*eta + n*(eta*cosi - sqrtf(k));
}
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
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, int depth=0) {
    Vec3f hit_p, normal;
    Material material;
    if (depth>max_depth||!intersect(orig, dir, hit_p, normal, material)) {
        return backgroud.color;
    }
    float diffuse_intensity = 0, specular_intensity = 0;
    Vec3f reflect_dir = reflect(dir, normal).normalize();
    Vec3f reflect_orig = reflect_dir*normal < 0 ? hit_p - normal*1e-3 : hit_p + normal*1e-3;
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, depth+1);
    Vec3f refract_dir = refract(dir, normal, material.refractive_index).normalize();
    Vec3f refract_orig = refract_dir*normal < 0 ? hit_p - normal*1e-3 : hit_p + normal*1e-3;
    Vec3f refract_color = cast_ray(refract_orig, refract_dir, depth+1);
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
    return material.color*diffuse_intensity*material.albedo[0] + Vec3f(1., 1., 1.)*specular_intensity*material.albedo[1] + reflect_color*material.albedo[2] + refract_color*material.albedo[3];
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

