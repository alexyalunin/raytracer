#pragma once

#include <triangle.h>
#include <material.h>
#include <sphere.h>

struct Object {
    static constexpr double kEps = 1e-9;
    const Material *material = nullptr;
    Triangle polygon;
    Triangle texture;
    Triangle normal;

    const Vector *GetNormal(size_t index) const {
        return &normal[index];
    }

    bool NormalExists() const {
        return !(std::fabs(normal[0][0]) < kEps && std::fabs(normal[0][1]) < kEps && std::fabs(normal[0][2]) < kEps &&
                 std::fabs(normal[1][0]) < kEps && std::fabs(normal[1][1]) < kEps && std::fabs(normal[1][2]) < kEps &&
                 std::fabs(normal[2][0]) < kEps && std::fabs(normal[2][1]) < kEps && std::fabs(normal[2][2]) < kEps); 
    }
};

struct SphereObject {
    const Material *material = nullptr;
    Sphere sphere;
};
