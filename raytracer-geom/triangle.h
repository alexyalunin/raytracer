#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(std::initializer_list<Vector> list) {
        size_t i = 0;
        for (const auto& x : list) {
            vertices_[i++] = x;
        }
    }

    Triangle() = default;

    double Area() const {
        return Length(CrossProduct(Vector(vertices_[0], vertices_[1]),
                                   Vector(vertices_[0], vertices_[2]))) /
               2;
    }

    const Vector& GetVertex(size_t ind) const {
        return vertices_[ind];
    }

    Vector& operator[](size_t ind) {
        return vertices_[ind];
    }

    const Vector& operator[](size_t ind) const {
        return vertices_[ind];
    }

private:
    std::array<Vector, 3> vertices_;
};
