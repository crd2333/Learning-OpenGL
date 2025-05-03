// 定义要渲染的场景。包括设置参数，物体以及灯光

#pragma once

#include <vector>
#include <memory>
#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"

class Scene {
public:
    // setting up options
    int width = 1280; // 屏幕的宽高
    int height = 960;
    double fov = 90;  // 角度制，需要转换为弧度制
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
    int maxDepth = 5;
    float epsilon = 0.00001;

    Scene(int w, int h) : width(w), height(h) {}

    // 用移动语义和智能指针添加物体和灯光（性能和安全考虑）
    // 另外这里用 object 这个基类做了个多态，可以添加 Sphere 和 MeshTriangle
    void Add(std::unique_ptr<Object> object) { objects.push_back(std::move(object)); }
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }

    [[nodiscard]] const std::vector<std::unique_ptr<Object>> &get_objects() const { return objects; }
    [[nodiscard]] const std::vector<std::unique_ptr<Light>> &get_lights() const { return lights; }

private:
    // creating the scene (adding objects and lights)
    std::vector<std::unique_ptr<Object> > objects;
    std::vector<std::unique_ptr<Light> > lights;
};