//
// Created by LEI XU on 5/16/19.
//
// 光线类，包含一条光的源头、方向、传递时间 t 和范围 range

#ifndef RAYTRACING_RAY_H
#define RAYTRACING_RAY_H
#include "Vector.hpp"

struct Ray {
    // Destination = origin + t*direction
    Vector3f origin;                   // 光源位置
    Vector3f direction, direction_inv; // 光线方向，存储 inverse direction 的原因是乘法比除法快，因此预先做好除法
    double t;  // transportation time，之后计算的时候用
    double t_min, t_max;

    Ray(const Vector3f &ori, const Vector3f &dir, const double _t = 0.0): origin(ori), direction(dir), t(_t) {
        direction_inv = Vector3f(1. / direction.x, 1. / direction.y, 1. / direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    Vector3f operator()(double t) const {return origin + direction * t;} // 重载了 () 运算符，根据 t 返回光线上某一点的坐标

    friend std::ostream &operator<<(std::ostream &os, const Ray &r) {
        os << "[origin:=" << r.origin << ", direction=" << r.direction << ", time=" << r.t << "]\n";
        return os;
    }
};
#endif //RAYTRACING_RAY_H
