#include <vector.h>
#include <sphere.h>
#include <intersection.h>
#include <triangle.h>
#include <ray.h>
#include <math.h>

#include <optional>

constexpr double kEps = 1e-9;

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Vector l = sphere.GetCenter() - ray.GetOrigin();
    double tc = DotProduct(l, ray.GetDirection());

    if (tc < 0.0) {
        return std::nullopt;
    }

    double d2 = Length(l) * Length(l) - tc * tc;

    double radius2 = sphere.GetRadius() * sphere.GetRadius();

    if (d2 > radius2) {
        return std::nullopt;
    }

    // solve for t1c
    double t1c = sqrt(radius2 - d2);

    // solve for intersection points
    double t1 = tc - t1c;
    double t2 = tc + t1c;

    auto position1 = ray.GetOrigin() + t1 * ray.GetDirection();
    auto position2 = ray.GetOrigin() + t2 * ray.GetDirection();
    Vector position;
    if (Length(ray.GetOrigin(), sphere.GetCenter()) < sphere.GetRadius()) {
        position = DotProduct(ray.GetDirection(), Vector(ray.GetOrigin(), position1)) > 0
                       ? position1
                       : position2;
    } else {
        position = Length(ray.GetOrigin(), position1) < Length(ray.GetOrigin(), position2)
                        ? position1
                        : position2;
    }
    auto normal = Length(ray.GetOrigin(), sphere.GetCenter()) < sphere.GetRadius()
                      ? Vector(position, sphere.GetCenter())
                      : Vector(sphere.GetCenter(), position);
    normal.Normalize();
    return Intersection(position, normal, Length(Vector(ray.GetOrigin(), position)));
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector e1 = triangle[1] - triangle[0];
    Vector e2 = triangle[2] - triangle[0];
    // Computing normal verctor to plane
    Vector pvec = CrossProduct(ray.GetDirection(), e2);
    double det = DotProduct(e1, pvec);

    // Ray is parallel to plane
    if (det < kEps && det > -kEps) {
        return std::nullopt;
    }

    double inv_det = 1 / det;
    Vector tvec = ray.GetOrigin() - triangle[0];
    double u = DotProduct(tvec, pvec) * inv_det;
    if (u < 0 || u > 1) {
        return std::nullopt;
    }

    Vector qvec = CrossProduct(tvec, e1);
    double v = DotProduct(ray.GetDirection(), qvec) * inv_det;
    if (v < 0 || u + v > 1) {
        return std::nullopt;
    }
    double k = DotProduct(e2, qvec) * inv_det;
    if (k < 0) {
        return std::nullopt;
    }
    auto position = ray.GetOrigin() + k * ray.GetDirection();
    Vector normal1 = CrossProduct(e1, e2);
    Vector normal2 = -1 * CrossProduct(e1, e2);
    normal1.Normalize();
    normal2.Normalize();
    Vector normal =
        Length(position + normal1, ray.GetOrigin()) < Length(position + normal2, ray.GetOrigin()) ? normal1 : normal2;
    normal.Normalize();
    return Intersection(position, normal, Length(ray.GetOrigin(), position));
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    double cos_theta_1 = -DotProduct(normal, ray);
    double cos_theta_2 = sqrt(1 - eta * eta * (1 - cos_theta_1 * cos_theta_1));
    return eta * ray + (eta * cos_theta_1 - cos_theta_2) * normal;
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    double cos_theta_1 = -DotProduct(normal, ray);
    Vector ans = ray + 2 * cos_theta_1 * normal;
    return ans;
}

double TriangleArea(const Triangle& triangle) {
    return std::abs(Length(
               CrossProduct(Vector(triangle[0], triangle[1]), Vector(triangle[0], triangle[2])))) /
           2;
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    Vector ans;
    double sum = 0;
    for (size_t i = 0; i != 3; ++i) {
        Triangle cur_triangle = triangle;
        cur_triangle[i] = point;
        ans[i] = TriangleArea(cur_triangle);
        sum += ans[i];
    }
    for (size_t i = 0; i != 3; ++i) {
        ans[i] /= sum;
    }
    return ans;
}
 