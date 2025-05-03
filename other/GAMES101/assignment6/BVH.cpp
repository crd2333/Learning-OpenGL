#include <algorithm>
#include <cassert>
#include "BVH.hpp"
// Bounding Volume Hierarch acceleration structure

// 构造函数，调用 recursiveBuild 并且计个时
BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode, SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p)) {
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    std::string SplitMethod = splitMethod == SplitMethod::NAIVE ? "NAIVE" : "SAH";
    std::cout << "\rBVH Generation complete: \nTime Taken: " << hrs << " hrs, " << mins << " mins, " << secs << " secs, " << "using " << SplitMethod << " method ...\n\n";
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects) {
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    } else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    } else {
        switch (splitMethod) {
        case SplitMethod::SAH : {
            auto size = objects.size();
            if (size >= 64) {  // 如果物体数量少，可能还是 NAIVE 有效一些？
                int middle_cut = 0;
                float min_time = std::numeric_limits<float>::max();

                // pre-compute the areas
                Bounds3 leftBounds, rightBounds;
                double leftBounds_Area[size], rightBounds_Area[size];

                for (int i = 0; i < size - 1; ++i) {
                    leftBounds = Union(leftBounds, objects[i]->getBounds().Centroid());
                    rightBounds = Union(rightBounds, objects[size - 1 - i]->getBounds().Centroid());
                    auto left_area = leftBounds.SurfaceArea();
                    auto right_area = rightBounds.SurfaceArea();
                    leftBounds_Area[i] = left_area / (left_area + right_area);
                    rightBounds_Area[i] = right_area / (left_area + right_area);
                }

                // find the best cut that minimizes the cost
                for (int i = 0; i < size - 1; ++i) {
                    float leftArea = leftBounds_Area[i];
                    float rightArea = rightBounds_Area[size - 2 - i];
                    auto time = (i + 1) * leftArea + (size - i - 1) * rightArea;
                    if (time < min_time) {
                        min_time = time;
                        middle_cut = i;
                    }
                }

                auto beginning = objects.begin();
                auto middling = objects.begin() + middle_cut;
                auto ending = objects.end();

                auto leftshapes = std::vector<Object*>(beginning, middling);
                auto rightshapes = std::vector<Object*>(middling, ending);
                assert(size == (leftshapes.size() + rightshapes.size()));

                node->left = recursiveBuild(leftshapes);
                node->right = recursiveBuild(rightshapes);

                node->bounds = Union(node->left->bounds, node->right->bounds);
                break;
            }
        }
        default: {
            Bounds3 centroidBounds;
            for (int i = 0; i < objects.size(); ++i)
                centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
            int dim = centroidBounds.maxExtent();
            switch (dim) { // 根据最长轴划分
            case 0:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x;
                });
                break;
            case 1:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y;
                });
                break;
            case 2:
                std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                    return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z;
                });
                break;
            }

            auto beginning = objects.begin();
            auto middling = objects.begin() + (objects.size() / 2);
            auto ending = objects.end();

            auto leftshapes = std::vector<Object*>(beginning, middling);
            auto rightshapes = std::vector<Object*>(middling, ending);

            assert(objects.size() == (leftshapes.size() + rightshapes.size()));

            node->left = recursiveBuild(leftshapes);
            node->right = recursiveBuild(rightshapes);

            node->bounds = Union(node->left->bounds, node->right->bounds);
        }
        }
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray &ray) const {
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray &ray) const {
    // TODO Traverse the BVH to find intersection
    std::array<int, 3> dirIsNeg = {ray.direction.x < 0, ray.direction.y < 0, ray.direction.z < 0};
    Intersection sect;
    if (!node->bounds.IntersectP(ray, dirIsNeg)) // if not intersect, directly return，其实不太需要？
        return sect;
    if (node->isLeaf()) { // or else, according to if it is a leaf to decide the next step
        return node->object->getIntersection(ray); // 每个子节点只会有一个 object，这是由聚合聚类的生成方式决定的
    } else { // 无子节点
        Intersection sect1 = getIntersection(node->left, ray);
        Intersection sect2 = getIntersection(node->right, ray);
        return sect1.distance < sect2.distance ? sect1 : sect2;
    }
}