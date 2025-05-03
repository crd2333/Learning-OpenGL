//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_SHADER_H
#define RASTERIZER_SHADER_H
#include <eigen3/Eigen/Eigen>
#include "Texture.hpp"

// 片元着色器（逐像素处理）
struct fragment_shader_payload {
    fragment_shader_payload() {
        texture = nullptr;
    }

    fragment_shader_payload(const Eigen::Vector3f &col, const Eigen::Vector3f &nor, const Eigen::Vector2f &tc, Texture* tex) :
        color(col), normal(nor), tex_coords(tc), texture(tex) {}

    fragment_shader_payload(const Eigen::Vector3f &vp, const Eigen::Vector3f &col, const Eigen::Vector3f &nor, const Eigen::Vector2f &tc, Texture* tex) :
        view_pos(vp), color(col), normal(nor), tex_coords(tc), texture(tex) {}

    Eigen::Vector3f view_pos;   // 视点位置（物体上某一点的位置）
    Eigen::Vector3f color;      // 颜色
    Eigen::Vector3f normal;     // 法向量
    Eigen::Vector2f tex_coords; // u, v 坐标
    Texture* texture;           // 纹理贴图指针
};

// 面元着色器
struct vertex_shader_payload {
    Eigen::Vector3f position;
};

#endif //RASTERIZER_SHADER_H
