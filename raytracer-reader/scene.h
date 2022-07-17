#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <vector>
#include <map>
#include <string>
#include <fstream>

class Scene {
public:
    const std::vector<Object>& GetObjects() const {
        return objects_;
    }

    void AddObject(const Object& object) {
        objects_.push_back(object);
    }

    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }

    void AddSphereObject(const SphereObject& sphere_object) {
        sphere_objects_.push_back(sphere_object);
    }

    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    void AddLight(const Light& light) {
        lights_.push_back(light);
    }

    const std::map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

    void SetMaterials(const std::map<std::string, Material>& materials) {
        materials_ = materials;
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::map<std::string, Material> materials_;
};

std::vector<std::string> Split(const std::string& string, const std::string& delimiter = " ") {
    size_t i = 0;
    std::vector<std::string> ans;
    while (true) {
        size_t found = std::string::npos;
        for (char c : delimiter) {
            found = std::min(found, string.find(c, i));
        }
        if (found == std::string::npos) {
            ans.push_back(string.substr(i, string.size() - i));
            if (ans.back().empty()) {
                ans.pop_back();
            }
            break;
        }
        ans.push_back(string.substr(i, found - i));
        if (ans.back().empty()) {
            ans.pop_back();
        }
        i = found + 1;
    }
    return ans;
}

std::vector<std::string> SplitSlash(const std::string& string, const std::string& delimiter = " ") {
    size_t i = 0;
    std::vector<std::string> ans;
    while (true) {
        size_t found = std::string::npos;
        for (char c : delimiter) {
            found = std::min(found, string.find(c, i));
        }
        if (found == std::string::npos) {
            ans.push_back(string.substr(i, string.size() - i));
            break;
        }
        ans.push_back(string.substr(i, found - i));
        i = found + 1;
    }
    return ans;
}

double ToDouble(const std::string& string) {
    return atof(string.c_str());
}

int ToInt(const std::string& string) {
    return atoi(string.c_str());
}

Vector ParseVectorPos(const std::vector<std::string>& splitted, size_t& i) {
    std::vector<double> ans;
    for (size_t j = 0; j != 3; ++j) {
        if (splitted[i++].empty()) {
            --j;
            continue;
        }
        ans.push_back(ToDouble(splitted[i - 1]));
    }
    return Vector(ans);
}

std::array<double, 3> ParseArrayPos(const std::vector<std::string>& splitted, size_t& i) {
    return {ToDouble(splitted[i++]), ToDouble(splitted[i++]), ToDouble(splitted[i++])};
}

Vector GetVectorByIndex(int index, const std::vector<Vector>& vectors) {
    if (index == 0) {
        return {0, 0, 0};
    }
    // std::cerr << vectors.size() << " " << index << "\n";
    if (index < 0) {
        return vectors.at(vectors.size() + index);
    } else {
        return vectors.at(index - 1);
    }
}

Triangle GetTriangleByIndex(int i, int j, int k, const std::vector<Vector>& vector,
                            const std::vector<int>& indices) {
    // std::cerr << "indices size " <<  indices.size() << "\n";
    // std::cerr << "get triangle by index" <<  indices[0] << " " << indices[i - 1] << " " << indices[i] << "\n";
    return {
        GetVectorByIndex(indices[i], vector),
        GetVectorByIndex(indices[j], vector),
        GetVectorByIndex(indices[k], vector),
    };
}

inline std::map<std::string, Material> ReadMaterials(std::string_view filename) {
    std::map<std::string, Material> materials;
    std::fstream file(filename.data(), std::fstream::in);
    std::string line;
    Scene scene;
    std::string current_material;

    while (std::getline(file, line)) {
        auto splitted = Split(line, " \t");

        if (!splitted.empty() && splitted[0] == "#") {
            continue;
        }

        for (size_t i = 0; i != splitted.size();) {
            const auto& string = splitted[i++];
            if (string == "newmtl") {
                current_material = splitted[i++];
                materials[current_material].name = current_material;
            } else if (string == "Ka") {
                materials[current_material].ambient_color = ParseVectorPos(splitted, i);
            } else if (string == "Kd") {
                materials[current_material].diffuse_color = ParseVectorPos(splitted, i);
            } else if (string == "Ks") {
                materials[current_material].specular_color = ParseVectorPos(splitted, i);
            } else if (string == "Ke") {
                materials[current_material].intensity = ParseVectorPos(splitted, i);
            } else if (string == "Ns") {
                materials[current_material].specular_exponent = ToDouble(splitted[i++]);
            } else if (string == "Ni") {
                materials[current_material].refraction_index = ToDouble(splitted[i++]);
            } else if (string == "al") {
                materials[current_material].albedo = ParseArrayPos(splitted, i);
            }
        }
    }
    return materials;
}

inline Scene ReadScene(std::string_view filename) {
    std::fstream file(filename.data(), std::fstream::in);
    std::string line;
    Scene scene;

    std::string current_material;

    std::vector<Vector> vertices;
    std::vector<Vector> textures;
    std::vector<Vector> normals;
    while (std::getline(file, line)) {
        auto splitted = Split(line, " \t");

        if (!splitted.empty() && splitted[0] == "#") {
            continue;
        }

        for (size_t i = 0; i != splitted.size();) {
            const auto& string = splitted[i++];
            if (string == "mtllib") {
                const std::string& path = filename.data();
                scene.SetMaterials(
                    ReadMaterials(path.substr(0, path.find_last_of('/') + 1) + splitted[i++]));
            } else if (string == "usemtl") {
                current_material = splitted[i++];
            } else if (string == "v") {
                vertices.push_back(ParseVectorPos(splitted, i));
            } else if (string == "vt") {
                textures.push_back(ParseVectorPos(splitted, i));
            } else if (string == "vn") {
                normals.push_back(ParseVectorPos(splitted, i));
            } else if (string == "S") {
                const auto& center = ParseVectorPos(splitted, i);
                double radius = ToDouble(splitted[i++]);
                scene.AddSphereObject(
                    {&scene.GetMaterials().at(current_material), Sphere(center, radius)});
            } else if (string == "P") {
                const auto& position = ParseVectorPos(splitted, i);
                const auto& intensity = ParseVectorPos(splitted, i);
                scene.AddLight({position, intensity});
            } else if (string == "f") {
                std::vector<int> vertex_indexes;
                std::vector<int> texture_indexes;
                std::vector<int> normal_indexes;
                while (i != splitted.size()) {
                    const auto& indices = SplitSlash(splitted[i++], "/");
                    if (indices.size() == 1) {
                        vertex_indexes.push_back(ToInt(indices[0]));
                        texture_indexes.push_back(0);
                        normal_indexes.push_back(0);
                    } else if (indices.size() == 2) {
                        vertex_indexes.push_back(ToInt(indices[0]));
                        texture_indexes.push_back(ToInt(indices[1]));
                        normal_indexes.push_back(0);
                    } else {
                        if (indices[1].empty()) {
                            vertex_indexes.push_back(ToInt(indices[0]));
                            texture_indexes.push_back(0);
                            normal_indexes.push_back(ToInt(indices[2]));
                        } else {
                            vertex_indexes.push_back(ToInt(indices[0]));
                            texture_indexes.push_back(ToInt(indices[1]));
                            normal_indexes.push_back(ToInt(indices[2]));
                        }
                    }
                }
                for (size_t j = 2; j != vertex_indexes.size(); ++j) {
                    scene.AddObject({
                        &scene.GetMaterials().at(current_material),
                        GetTriangleByIndex(0, j - 1, j, vertices, vertex_indexes),
                        GetTriangleByIndex(0, j - 1, j, textures, texture_indexes),
                        GetTriangleByIndex(0, j - 1, j, normals, normal_indexes),
                    });
                }
            }
        }
    }
    return scene;
}
