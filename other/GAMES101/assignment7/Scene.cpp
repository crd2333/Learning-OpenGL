//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
    std::cout << " - Generating BVH ...\n\n";
}

Intersection Scene::intersect(const Ray &ray) const {
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
    float emit_area_sum = 0; // 发光区域的总面积
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit())
            emit_area_sum += objects[k]->getArea();
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) { // 再遍历一遍，当第一次发生发光区域之和大于 p 时，就采样这个物体（有点无语的算法。。。
        if (objects[k]->hasEmit()) {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum) {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object*> &objects,
    float &tNear, uint32_t &index, Object** hitObject) {
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
    // 从像素打出的 ray 与 scene 中物体求交（将会调用 BVH 进行加速）
    Intersection inter = intersect(ray);
    // 交点的一些信息
    Vector3f p = inter.coords;
    Vector3f N = inter.normal.normalized();
    Vector3f wo = ray.direction; // 指向交点
    Material* m = inter.m;

    if (!inter.happened)        // case1: 没有打到任何物体，直接返回背景色
        return this->backgroundColor;
        // return this->backgroundColor;
    else if (m->hasEmission())  // case2: 打到了发光物体，根据渲染方程 Lo(p, wo) = Le(p, wo) + xxx，直接返回发光物体的颜色
        return m->getEmission();
    else {                      // case3: 打到普通物体，开始执行伪代码算法
        // L_direct: 直接对 light 采样以减少胡乱采样做的无用功
        Vector3f L_dir(0.0f);
        float pdf_light = 0;
        Intersection inter2;
        sampleLight(inter2, pdf_light); // 通过采样得到如果没有被阻挡则应该的相交的点和 pdf
        // 应该交点的一些信息
        Vector3f pp = inter2.coords;
        Vector3f NN = inter2.normal.normalized();
        Vector3f ws = (pp - p).normalized(); // p to pp
        Vector3f emit = inter2.m->getEmission();
        double distance = (p - pp).norm();
        double distance2 = pow(distance, 2);
        if (Intersection light_pos = intersect(Ray(p, ws));
            light_pos.happened && distance - (light_pos.coords - p).norm() < EPSILON)  // 没有被阻挡
            L_dir = emit * m->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) / distance2 / pdf_light;

        // L_indirect: 根据俄罗斯轮盘赌概率决定是否继续随机投射
        Vector3f L_indir(0.0f);
        if (!testRR())
            return L_dir;
        Vector3f wi = m->sample(wo, N).normalized(); // 根据材质决定反射、漫反射等，采样出一个方向（目前是只有漫反射）
        Ray r(p, wi);
        if (inter2 = intersect(r); inter2.happened) { // 如果有撞到物体
            // 交点的一些信息
            pp = inter2.coords;
            NN = inter2.normal.normalized();
            ws = (pp - p).normalized(); // p to pp
            if (!inter2.m->hasEmission())  // 如果撞到的物体不是发光物体
                L_indir = castRay(r, depth + 1) * m->eval(wo, wi, N) * dotProduct(wi, N) / m->pdf(wo, wi, N) / this->RussianRoulette;
        }
        return L_dir + L_indir;
    }
}