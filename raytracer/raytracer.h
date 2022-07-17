#pragma once

#include <image.h>
#include <camera_options.h>
#include <render_options.h>
#include <string>
#include <scene.h>
#include <geometry.h>

std::vector<std::vector<Vector>> GetRayDirs(const CameraOptions& camera_options) {
    std::vector<std::vector<Vector>> ans(camera_options.screen_width,
                                         std::vector<Vector>(camera_options.screen_height));

    Vector forward = Vector(camera_options.look_from) - Vector(camera_options.look_to);
    forward.Normalize();
    Vector right = CrossProduct({0, 1, 0}, forward);
    if (1 - std::fabs(forward[1]) < kEps) {
        right = {1, 0, 0};
    }
    right.Normalize();
    Vector up = CrossProduct(forward, right);
    up.Normalize();

    const Vector& u = right;
    const Vector& v = up;
    const Vector& w = forward;

    double scale = std::tan(camera_options.fov / 2);
    double image_aspect_ratio = 1.0 * camera_options.screen_width / camera_options.screen_height;
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            double x =
                (2 * (i + 0.5) / camera_options.screen_width - 1) * image_aspect_ratio * scale;
            double y = (2 * (j + 0.5) / camera_options.screen_height - 1) * scale;
            Vector t = {x, -y, -1};
            t.Normalize();
            ans[i][j] = t[0] * u + t[1] * v + t[2] * w;
            ans[i][j].Normalize();
        }
    }

    return ans;
}

Image RenderDepth(const std::string& filename, const CameraOptions& camera_options) {
    const auto& scene = ReadScene(filename);
    Image image(camera_options.screen_width, camera_options.screen_height);
    std::vector<std::vector<Vector>> ray_dirs = GetRayDirs(camera_options);
    std::vector<std::vector<double>> ans(camera_options.screen_width,
                                         std::vector<double>(camera_options.screen_height, -1));
    double max = 0;
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_dirs[i][j]);
            double dst;
            bool updated = false;
            for (const auto& object : scene.GetObjects()) {
                auto intersection = GetIntersection(ray, object.polygon);
                if (!intersection) {
                    continue;
                }
                if (!updated) {
                    dst = Length(ray.GetOrigin(), intersection->GetPosition());
                    updated = true;
                } else {
                    dst = std::min(dst, Length(ray.GetOrigin(), intersection->GetPosition()));
                }
            }
            for (const auto& object : scene.GetSphereObjects()) {
                auto intersection = GetIntersection(ray, object.sphere);
                if (!intersection) {
                    continue;
                }
                if (!updated) {
                    dst = Length(ray.GetOrigin(), intersection->GetPosition());
                    updated = true;
                } else {
                    dst = std::min(dst, Length(ray.GetOrigin(), intersection->GetPosition()));
                }
            }
            if (updated) {
                ans[i][j] = dst;
                max = std::max(max, dst);
            }
        }
    }
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            double val = 255.0 / 256;
            if (ans[i][j] > 0) {
                val = ans[i][j] / max;
            }
            val *= 256;
            int ans = val;
            image.SetPixel({ans, ans, ans}, i, j);
        }
    }
    return image;
}

Image RenderNormal(const std::string& filename, const CameraOptions& camera_options) {
    const auto& scene = ReadScene(filename);
    Image image(camera_options.screen_width, camera_options.screen_height);
    std::vector<std::vector<Vector>> ray_dirs = GetRayDirs(camera_options);
    std::vector<std::vector<Vector>> ans(
        camera_options.screen_width,
        std::vector<Vector>(camera_options.screen_height, {-100, -100, -100}));
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_dirs[i][j]);
            double dst;
            bool updated = false;
            Vector normal;

            for (const auto& object : scene.GetObjects()) {
                auto intersection = GetIntersection(ray, object.polygon);
                if (!intersection) {
                    continue;
                }
                if (!updated) {
                    dst = Length(ray.GetOrigin(), intersection->GetPosition());
                    if (!object.NormalExists()) {
                        normal = intersection->GetNormal();
                    } else {
                        normal = {0, 0, 0};
                        Vector barycentric =
                            GetBarycentricCoords(object.polygon, intersection->GetPosition());
                        for (int i = 0; i != 3; ++i) {
                            normal = normal + barycentric[i] * object.normal[i];
                        }
                    }
                    updated = true;
                } else {
                    if (Length(ray.GetOrigin(), intersection->GetPosition()) < dst) {
                        if (!object.NormalExists()) {
                            normal = intersection->GetNormal();
                        } else {
                            normal = {0, 0, 0};
                            Vector barycentric =
                                GetBarycentricCoords(object.polygon, intersection->GetPosition());
                            for (int i = 0; i != 3; ++i) {
                                normal = normal + barycentric[i] * object.normal[i];
                            }
                        }
                    }
                    dst = std::min(dst, Length(ray.GetOrigin(), intersection->GetPosition()));
                }
            }

            for (const auto& object : scene.GetSphereObjects()) {
                auto intersection = GetIntersection(ray, object.sphere);
                if (!intersection) {
                    continue;
                }
                if (!updated) {
                    dst = Length(ray.GetOrigin(), intersection->GetPosition());
                    normal = intersection->GetNormal();
                    updated = true;
                } else {
                    if (Length(ray.GetOrigin(), intersection->GetPosition()) < dst) {
                        normal = intersection->GetNormal();
                    }
                    dst = std::min(dst, Length(ray.GetOrigin(), intersection->GetPosition()));
                }
            }
            if (updated) {
                ans[i][j] = normal;
            }
        }
    }
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            if (ans[i][j][0] < -99) {
                image.SetPixel({0, 0, 0}, i, j);
                continue;
            }
            Vector cur = 255 * (0.5 * ans[i][j] + Vector{0.5, 0.5, 0.5});
            image.SetPixel(
                {static_cast<int>(cur[0]), static_cast<int>(cur[1]), static_cast<int>(cur[2])}, i,
                j);
        }
    }
    return image;
}

constexpr double kErrSame = 1e-6;

bool HasIntersections(const Scene& scene, const Ray& ray, double len) {
    for (const auto& object : scene.GetObjects()) {
        auto cur_intersection = GetIntersection(ray, object.polygon);
        if (cur_intersection &&
            len + 1e-5 > Length(ray.GetOrigin(), cur_intersection->GetPosition())) {
            return true;
        }
    }
    for (const auto& object : scene.GetSphereObjects()) {
        auto cur_intersection = GetIntersection(ray, object.sphere);
        if (cur_intersection &&
            len + 1e-5 > Length(ray.GetOrigin(), cur_intersection->GetPosition())) {
            return true;
        }
    }
    return false;
}

Vector CalculateBase(const Scene& scene, const Intersection& intersection, const Material& material,
                     const Vector& normal, const Vector& from) {
    Vector ans{0, 0, 0};
    ans = ans + material.ambient_color;
    ans = ans + material.intensity;
    for (const auto& light : scene.GetLights()) {
        Vector dir(intersection.GetPosition(), light.position);
        dir.Normalize();
        double len = Length(intersection.GetPosition(), light.position);
        if (HasIntersections(scene, Ray(intersection.GetPosition() + kErrSame * normal, dir),
                             len)) {
            continue;
        }
        Vector v_l(intersection.GetPosition(), light.position);
        v_l.Normalize();
        ans = ans + material.albedo[0] * std::max(0.0, DotProduct(normal, v_l)) *
                        material.diffuse_color * light.intensity;
        Vector v_e(intersection.GetPosition(), from);
        v_e.Normalize();
        ans = ans + material.albedo[0] *
                        std::pow(std::max(0.0, DotProduct(v_e, Reflect(-1. * v_l, normal))),
                                 material.specular_exponent) *
                        material.specular_color * light.intensity;
    }
    return ans;
}

Vector GetNormal(const Intersection& intersection, const Object& object) {
    if (!object.NormalExists()) {
        return intersection.GetNormal();
    } else {
        Vector normal = {0, 0, 0};
        Vector barycentric = GetBarycentricCoords(object.polygon, intersection.GetPosition());
        for (int i = 0; i != 3; ++i) {
            normal = normal + barycentric[i] * object.normal[i];
        }
        return normal;
    }
}

Vector GetNormal(const Intersection& intersection, const SphereObject&) {
    return intersection.GetNormal();
}

Vector Cast(const Scene& scene, const Ray& ray, const RenderOptions& render_options,
            bool inside = false, int depth = 0) {
    if (depth == render_options.depth) {
        return {0, 0, 0};
    }
    double dst;
    bool updated = false;
    Vector normal;

    const Material* material;
    Intersection intersection;

    for (const auto& object : scene.GetObjects()) {
        auto cur_intersection = GetIntersection(ray, object.polygon);
        if (!cur_intersection) {
            continue;
        }
        if (!updated || Length(ray.GetOrigin(), cur_intersection->GetPosition()) < dst) {
            dst = Length(ray.GetOrigin(), cur_intersection->GetPosition());
            normal = GetNormal(*cur_intersection, object);
            updated = true;
            material = object.material;
            intersection = *cur_intersection;
        }
    }

    for (const auto& object : scene.GetSphereObjects()) {
        auto cur_intersection = GetIntersection(ray, object.sphere);
        if (!cur_intersection) {
            continue;
        }
        if (!updated || Length(ray.GetOrigin(), cur_intersection->GetPosition()) < dst) {
            dst = Length(ray.GetOrigin(), cur_intersection->GetPosition());
            normal = GetNormal(*cur_intersection, object);
            updated = true;
            material = object.material;
            intersection = *cur_intersection;
        }
    }

    if (!updated) {
        return {0, 0, 0};
    }

    auto cur_vec = Vector(ray.GetOrigin(), intersection.GetPosition());
    cur_vec.Normalize();
    Vector reflected = Reflect(cur_vec, normal);

    if (inside) {
        Vector refracted = *Refract(cur_vec, normal, material->refraction_index);
        return CalculateBase(scene, intersection, *material, normal, ray.GetOrigin()) +
               (material->albedo[1] + material->albedo[2]) *
                   Cast(scene, Ray(intersection.GetPosition() - kErrSame * normal, refracted),
                        render_options, !inside, depth + 1);
    } else {
        Vector refracted = *Refract(cur_vec, normal, 1 / material->refraction_index);
        return CalculateBase(scene, intersection, *material, normal, ray.GetOrigin()) +
               material->albedo[1] *
                   Cast(scene, Ray(intersection.GetPosition() + kErrSame * normal, reflected),
                        render_options, false, depth + 1) +
               material->albedo[2] *
                   Cast(scene, Ray(intersection.GetPosition() - kErrSame * normal, refracted),
                        render_options, !inside, depth + 1);
    }
}

void PostProcess(std::vector<std::vector<Vector>>& ans) {
    double c = 0;
    for (size_t i = 0; i != ans.size(); ++i) {
        for (size_t j = 0; j != ans[0].size(); ++j) {
            for (size_t k = 0; k != 3; ++k) {
                c = std::max(c, ans[i][j][k]);
            }
        }
    }
    for (size_t i = 0; i != ans.size(); ++i) {
        for (size_t j = 0; j != ans[0].size(); ++j) {
            ans[i][j] = ans[i][j] * (Vector(1., 1., 1.) + ans[i][j] / Vector(c * c, c * c, c * c)) /
                        (Vector(1., 1., 1.) + ans[i][j]);
        }
    }
}

void GammaCorrection(std::vector<std::vector<Vector>>& ans) {
    for (size_t i = 0; i != ans.size(); ++i) {
        for (size_t j = 0; j != ans[0].size(); ++j) {
            for (size_t k = 0; k != 3; ++k) {
                ans[i][j][k] = std::pow(ans[i][j][k], 1 / 2.2);
            }
        }
    }
}

Image RenderFull(const std::string& filename, const CameraOptions& camera_options,
                 const RenderOptions& render_options) {
    const auto& scene = ReadScene(filename);
    Image image(camera_options.screen_width, camera_options.screen_height);
    std::vector<std::vector<Vector>> ray_dirs = GetRayDirs(camera_options);
    std::vector<std::vector<Vector>> ans(camera_options.screen_width,
                                         std::vector<Vector>(camera_options.screen_height));
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_dirs[i][j]);
            ans[i][j] = Cast(scene, ray, render_options);
        }
    }
    PostProcess(ans);
    GammaCorrection(ans);
    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            Vector cur = 255 * ans[i][j];
            image.SetPixel(
                {static_cast<int>(cur[0]), static_cast<int>(cur[1]), static_cast<int>(cur[2])}, i,
                j);
        }
    }
    return image;
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    if (render_options.mode == RenderMode::kDepth) {
        return RenderDepth(filename, camera_options);
    } else if (render_options.mode == RenderMode::kNormal) {
        return RenderNormal(filename, camera_options);
    } else {
        return RenderFull(filename, camera_options, render_options);
    }
}