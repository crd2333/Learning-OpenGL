//
// Created by LEI XU on 5/16/19.
//
// 包围盒类，每个包围盒可由 pMin 和 pMax 两点描述（请思考为什么）。因为包围盒是个立方盒子，只需要体对角线上两个点就可以确定
// 与材质一样，场景中的每个物体实例都有自己的包围盒

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H
#include "Ray.hpp"
#include "Vector.hpp"
#include <limits>
#include <array>

class Bounds3 {
public:
    Vector3f pMin, pMax; // two points to specify the bounding box
    Bounds3() {
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3f(minNum, minNum, minNum); // pMax 初始化为最小值，而 pMin 初始化为最大值
        pMin = Vector3f(maxNum, maxNum, maxNum);
    }
    Bounds3(const Vector3f p) : pMin(p), pMax(p) {}
    Bounds3(const Vector3f p1, const Vector3f p2) {
        pMin = Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
        pMax = Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
    }

    Vector3f Diagonal() const { return pMax - pMin; }
    int maxExtent() const { // 算出 xyz 三个轴最大的分量
        Vector3f d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0; // x 最大
        else if (d.y > d.z)
            return 1; // y 最大
        else
            return 2; // z 最大
    }

    double SurfaceArea() const { // 表面积
        Vector3f d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    Vector3f Centroid() { return 0.5 * pMin + 0.5 * pMax; } // 中心点

    Bounds3 Intersect(const Bounds3 &b) { // 两个包围盒的交集
        return Bounds3(Vector3f(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
                                fmax(pMin.z, b.pMin.z)),
                       Vector3f(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
                                fmin(pMax.z, b.pMax.z)));
    }

    Vector3f Offset(const Vector3f &p) const { // 某个点 p 相对 pMin 的偏移量，相对于包围盒长宽高的倍数
        Vector3f o = p - pMin;
        if (pMax.x > pMin.x) // 如果有合法值（非 lowest 和 max）
            o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

    bool Overlaps(const Bounds3 &b1, const Bounds3 &b2) { // 有重叠
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

    bool Inside(const Vector3f &p, const Bounds3 &b) { // 在内部
        return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
    }
    inline const Vector3f &operator[](int i) const {
        return (i == 0) ? pMin : pMax;
    }

    inline bool IntersectP(const Ray &ray, const Vector3f &invDir,
                           const std::array<int, 3> &dirisNeg) const;
    inline bool IntersectP(const Ray &ray, const std::array<int, 3> &dirisNeg) const;
};

inline bool Bounds3::IntersectP(const Ray &ray, const Vector3f &invDir,
                                const std::array<int, 3> &dirIsNeg) const {
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // TODO: test if ray bound intersects
    // 事实上这个 invDir 不用做参数，因为 ray.direction_inv 已经有这玩意儿了，自己新写了一个
    Vector3f tmin = (pMin - ray.origin) * invDir;
    Vector3f tmax = (pMax - ray.origin) * invDir;
    if (dirIsNeg[0])
        std::swap(tmin.x, tmax.x);
    if (dirIsNeg[1])
        std::swap(tmin.y, tmax.y);
    if (dirIsNeg[2])
        std::swap(tmin.z, tmax.z);
    float tenter = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float texit = std::min(std::min(tmax.x, tmax.y), tmax.z);
    return tenter < texit && texit >= 0;
}
inline bool Bounds3::IntersectP(const Ray &ray, const std::array<int, 3> &dirIsNeg) const {
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // TODO: test if ray bound intersects
    Vector3f tmin = (pMin - ray.origin) * ray.direction_inv;
    Vector3f tmax = (pMax - ray.origin) * ray.direction_inv;
    if (dirIsNeg[0])
        std::swap(tmin.x, tmax.x);
    if (dirIsNeg[1])
        std::swap(tmin.y, tmax.y);
    if (dirIsNeg[2])
        std::swap(tmin.z, tmax.z);
    float tenter = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float texit = std::min(std::min(tmax.x, tmax.y), tmax.z);
    return tenter < texit && texit >= 0;
}

// Bounds3::Union 函数的作用是将两个包围盒并成更大的包围盒，依然是立方体
inline Bounds3 Union(const Bounds3 &b1, const Bounds3 &b2) {
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b1.pMin, b2.pMin);
    ret.pMax = Vector3f::Max(b1.pMax, b2.pMax);
    return ret;
}

// 这个则是将一个包围盒和一个点并成更大的包围盒
inline Bounds3 Union(const Bounds3 &b, const Vector3f &p) {
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b.pMin, p);
    ret.pMax = Vector3f::Max(b.pMax, p);
    return ret;
}

#endif // RAYTRACING_BOUNDS3_H
