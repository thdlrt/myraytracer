//
// Created by MSI on 24-4-7.
//

#ifndef SPHERE_H
#define SPHERE_H

struct PointLight {
    Vec3f position;
    float intensity;
    PointLight(const Vec3f p, const float i) : position(p), intensity(i) {}
};

struct Material {
    Vec3f color;
    Material(const Vec3f &color) : color(color) {}
    Material() : color() {}
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material m) : center(c), radius(r), material(m){}
    /*@param orig: origin of the ray
      @param dir: direction of the ray
      @param t0: distance from the origin to the first intersection point*/
    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

#endif //SPHERE_H
