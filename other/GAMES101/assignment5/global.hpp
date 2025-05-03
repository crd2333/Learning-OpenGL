#pragma once

#include <cmath>
#include <iostream>
#include <random>

#define M_PI 3.14159265358979323846

constexpr float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v) { // 将 v 限制在 [lo, hi] 之间
    return std::max(lo, std::min(hi, v));
}

// 求解二次方程，通过返回值指示是否有解，通过引用参数返回得到的解
inline bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1) {
    float discr = b * b - 4 * a * c; // 判别式，Delta
    if (discr < 0)
        return false;
    else if (discr == 0)
        x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1)
        std::swap(x0, x1);
    return true;
}

enum MaterialType {
    DIFFUSE_AND_GLOSSY,         // diffuse and glossy，漫反射和高光，可以看作粗糙反射（介于镜面反射和漫反射之间）与漫反射的叠加
    REFLECTION_AND_REFRACTION,  // reflection and refraction，反射和折射，可以看作镜面反射和折射的叠加
    REFLECTION                  // reflection only，仅反射
};

inline float get_random_float() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6] // should be [0, 1)

    return dist(rng);
}

inline void UpdateProgress(float progress) { // 输出进度条
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}
