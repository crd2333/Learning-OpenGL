//
// Created by goksu on 4/6/19.
//

#pragma once

#include "Triangle.hpp"
#include <algorithm>
#include <eigen3/Eigen/Eigen>
using namespace Eigen;

namespace rst {
enum class Buffers { // 两种 buffer 类型
    Color = 1,
    Depth = 2
};

inline Buffers operator|(Buffers a, Buffers b) {
    return Buffers((int)a | (int)b);
}

inline Buffers operator&(Buffers a, Buffers b) {
    return Buffers((int)a & (int)b);
}

enum class Primitive {
    Line,
    Triangle
};

/*
 * For the curious : The draw function takes two buffer id's as its arguments.
 * These two structs make sure that if you mix up with their orders, the
 * compiler won't compile it. Aka : Type safety
 * */
struct pos_buf_id {
    int pos_id = 0;
};

struct ind_buf_id {
    int ind_id = 0;
};

class rasterizer {
public:
    rasterizer(int w, int h);
    pos_buf_id load_positions(const std::vector<Eigen::Vector3f> &positions);
    ind_buf_id load_indices(const std::vector<Eigen::Vector3i> &indices);

    void set_model(const Eigen::Matrix4f &m); // 将内部的模型矩阵作为参数传递给光栅化器
    void set_view(const Eigen::Matrix4f &v);  // 将视图变换矩阵设为内部视图矩阵
    void set_projection(const Eigen::Matrix4f &p); // 将内部的投影矩阵设为给定矩阵 p，并传递给光栅化器

    void set_pixel(const Eigen::Vector3f &point, const Eigen::Vector3f &color);

    void clear(Buffers buff); // 清空帧缓冲区或深度缓冲区

    void draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, Primitive type);

    std::vector<Eigen::Vector3f> &frame_buffer() { return frame_buf; }

private:
    void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);
    void rasterize_wireframe(const Triangle &t);

private:
    // 三个变换矩阵
    Eigen::Matrix4f model;
    Eigen::Matrix4f view;
    Eigen::Matrix4f projection;

    std::map<int, std::vector<Eigen::Vector3f>> pos_buf; // map for points' position buffer and index buffer
    std::map<int, std::vector<Eigen::Vector3i>> ind_buf;

    // 对某一帧，700 * 700 的数据
    std::vector<Eigen::Vector3f> frame_buf; // 帧缓冲对象，用于存储需要在屏幕上绘制的颜色数据
    std::vector<float> depth_buf;           // 深度缓冲对象，用于存储每个像素的深度值（现在好像没什么用）
    int get_index(int x, int y); // 根据 x 和 y 坐标计算出该帧中的一维索引值

    int width, height;

    int next_id = 0;
    int get_next_id() { return next_id++; }
};
} // namespace rst
