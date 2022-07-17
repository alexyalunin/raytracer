#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <algorithm>

class Vector;

inline double Length(const Vector& vec);

class Vector {
public:
    Vector() = default;

    Vector(std::initializer_list<double> list) {
        size_t i = 0;
        for (auto x : list) {
            data_[i++] = x;
        }
    }

    Vector(std::array<double, 3> data) : data_(data) {
    }

    Vector(std::vector<double> data) {
        size_t i = 0;
        for (auto x : data) {
            data_[i++] = x;
        }
    }

    Vector(double a, double b, double c) : Vector({a, b, c}) {
    }

    Vector(const Vector& a, const Vector& b) : Vector({b[0] - a[0], b[1] - a[1], b[2] - a[2]}) {
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }

    double operator[](size_t ind) const {
        return data_[ind];
    }

    void Normalize() {
        double length = Length(*this);
        for (auto& x : data_) {
            x /= length;
        }
    }

    friend Vector operator+(const Vector& a, const Vector& b) {
        return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
    }

    friend Vector operator-(const Vector& a, const Vector& b) {
        return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
    }

    friend Vector operator*(double k, const Vector& b) {
        return {k * b[0], k * b[1], k * b[2]};
    }

    friend Vector operator*(const Vector& a, const Vector& b) {
        return {a[0] * b[0], a[1] * b[1], a[2] * b[2]};
    }

    friend Vector operator/(const Vector& a, const Vector& b) {
        return {a[0] / b[0], a[1] / b[1], a[2] / b[2]};
    }

private:
    std::array<double, 3> data_;
};

inline double DotProduct(const Vector& lhs, const Vector& rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

inline Vector CrossProduct(const Vector& a, const Vector& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

inline double Length(const Vector& vec) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

inline double Length(const Vector& a, const Vector& b) {
    return Length(Vector(a, b));
}

std::ostream& operator<<(std::ostream& out, const Vector& a) {
    out << a[0] << " " << a[1] << " " << a[2];
    return out;
}
