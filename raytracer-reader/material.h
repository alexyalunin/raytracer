#pragma once

#include <vector.h>
#include <string>

struct Material {
    std::string name;
    Vector ambient_color = {0, 0, 0};
    Vector diffuse_color = {0, 0, 0};
    Vector specular_color = {0, 0, 0};
    Vector intensity = {0, 0, 0};
    double specular_exponent;
    double refraction_index;
    std::array<double, 3> albedo = {1, 0, 0};
};
