//
// Created by LEI XU on 5/16/19.
//
// 这个数据结构包含了相交相关的信息，即相交算法返回的结果

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection {
    Intersection() {
        happened = false;
        coords = Vector3f();
        normal = Vector3f();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }
    bool happened;
    Vector3f coords; // 交点坐标，即 o + t * d
    Vector3f normal;
    double distance;
    Object* obj;
    Material* m;
};
#endif //RAYTRACING_INTERSECTION_H
