//
// Created by LEI XU on 5/16/19.
//
//  BVH 加速类。场景 scene 拥有一个 BVHAccel 实例。从根节点开始，我们可以递归地从物体列表构造场景的 BVH.
// 课上讲的 BVH 是宏观上的概念，没有细讲其实现，可以看这篇博客： https://www.cnblogs.com/lookof/p/3546320.html

#ifndef RAYTRACING_BVH_H
#define RAYTRACING_BVH_H

#include <atomic>
#include <vector>
#include <memory>
#include <ctime>
#include "Object.hpp"
#include "Ray.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"

struct BVHBuildNode;
// BVHAccel Forward Declarations
struct BVHPrimitiveInfo;

// BVHAccel Declarations
inline int leafNodes, totalLeafNodes, totalPrimitives, interiorNodes;
class BVHAccel {
public:
    // BVHAccel Public Types
    enum class SplitMethod { NAIVE, SAH }; // SAH: Surface Area Heuristic

    // BVHAccel Public Methods
    BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);
    // Bounds3 WorldBound() const; // 没有实现？自己写了一个
    Bounds3 WorldBound(std::vector<Object*> objects) const {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
        return centroidBounds;
    }
    ~BVHAccel() = default;

    Intersection Intersect(const Ray &ray) const;
    Intersection getIntersection(BVHBuildNode* node, const Ray &ray)const;
    bool IntersectP(const Ray &ray) const; // bound3 intersectP，这个就不用了，在 BVHAccel 中用 getIntersection
    BVHBuildNode* root;

    const SplitMethod getSplitMethod() const { return splitMethod; }
private:
    // BVHAccel Private Methods
    BVHBuildNode* recursiveBuild(std::vector<Object*>objects);

    // BVHAccel Private Data
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<Object*> primitives;
};

struct BVHBuildNode {
    Bounds3 bounds;
    BVHBuildNode* left;
    BVHBuildNode* right;
    Object* object; // 该 node 包含的物体

public:
    int splitAxis = 0, firstPrimOffset = 0, nPrimitives = 0;  // 目前没用起来
    // BVHBuildNode Public Methods
    BVHBuildNode() {
        bounds = Bounds3();
        left = nullptr;
        right = nullptr;
        object = nullptr;
    }
    bool isLeaf() const { return left == nullptr && right == nullptr && object != nullptr; } // self-added
};


#endif //RAYTRACING_BVH_H
