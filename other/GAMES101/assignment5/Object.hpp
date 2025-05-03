// 渲染物体的父类。Triangle 和 Sphere 类都是从该类继承的

#pragma once

#include "Vector.hpp"
#include "global.hpp"

class Object {
public:
    Object() : materialType(DIFFUSE_AND_GLOSSY), ior(1.3), Kd(0.8), Ks(0.2), diffuseColor(0.2), specularExponent(25)
    {}

    virtual ~Object() = default;

    // orig, dir, tnear, index, uv，前面两个输入，后面三个输出
    virtual bool intersect(const Vector3f &, const Vector3f &, float &, uint32_t &, Vector2f &) const = 0;

    virtual void getSurfaceProperties(const Vector3f &, const Vector3f &, const uint32_t &, const Vector2f &, Vector3f &, Vector2f &) const = 0;

    virtual Vector3f evalDiffuseColor(const Vector2f &) const {
        return diffuseColor;
    }

    // material properties
    MaterialType materialType;
    float ior;              // 折射率
    float Kd, Ks;
    Vector3f diffuseColor;  // 用于 bling-phone 模型，如果是三角形则插值一下，如果是球体则直接返回默认值（成员变量）
    float specularExponent; // 镜面反射指数
};
