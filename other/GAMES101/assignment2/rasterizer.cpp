// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions) {
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices) {
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols) {
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f &v3, float w = 1.0f) {
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static float cross2D(const Vector2f& v1, const Vector2f& v2) {
    return v1.x()*v2.y() - v1.y()*v2.x();
}
static bool insideTriangle(int x, int y, const Vector3f* _v) {
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    // Eigen::Vector3f AB = _v[1] - _v[0];
    // Eigen::Vector3f BC = _v[2] - _v[1];
    // Eigen::Vector3f CA = _v[0] - _v[2];
    // Eigen::Vector3f AP = Eigen::Vector3f(x, y, 0) - _v[0];
    // Eigen::Vector3f BP = Eigen::Vector3f(x, y, 0) - _v[1];
    // Eigen::Vector3f CP = Eigen::Vector3f(x, y, 0) - _v[2];
    // return (AB.cross(AP).z() > 0 && BC.cross(BP).z() > 0 && CA.cross(CP).z() > 0) ||
    //        (AB.cross(AP).z() < 0 && BC.cross(BP).z() < 0 && CA.cross(CP).z() < 0);
    Vector2f p(x,y);
    bool ccw1 = cross2D(_v[1].head<2>() - _v[0].head<2>(), p - _v[0].head<2>()) >= 0;
    bool ccw2 = cross2D(_v[2].head<2>() - _v[1].head<2>(), p - _v[1].head<2>()) >= 0;
    bool ccw3 = cross2D(_v[0].head<2>() - _v[2].head<2>(), p - _v[2].head<2>()) >= 0;
    return ccw1 == ccw2 && ccw2 == ccw3;
}

// 计算 2D 平面中点 (x, y) 相对三角形的重心坐标
static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v) {
    float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
    float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
    float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
    return {c1, c2, c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type) {
    auto &buf = pos_buf[pos_buffer.pos_id];
    auto &ind = ind_buf[ind_buffer.ind_id];
    auto &col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto &i : ind) {
        Triangle t;
        Eigen::Vector4f v[] = {
            mvp * to_vec4(buf[i[0]], 1.0f),
            mvp * to_vec4(buf[i[1]], 1.0f),
            mvp * to_vec4(buf[i[2]], 1.0f)
        };
        // Homogeneous division
        for (auto &vec : v)
            vec /= vec.w();
        // Viewport transformation
        for (auto &vert : v) {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        // 还是一样的问题，设置三次是有什么心事吗？
        for (int i = 0; i < 3; ++i) {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle &t) {
    auto v = t.toVector4();

    // TODO : Find out the bounding box of current triangle.
    float xmin = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
    float xmax = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
    float ymin = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
    float ymax = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));
    xmin = std::max(0.0f, xmin);
    xmax = std::min((float)width - 1, xmax);
    ymin = std::max(0.0f, ymin);
    ymax = std::min((float)height - 1, ymax);

    // only one color per triangle now
    auto color = t.getColor();

    // iterate through the pixel and find if the current pixel is inside the triangle
    for (auto x = xmin; x <= xmax; x++) {
        for (auto y = ymin; y <= ymax; y++) {
            // If so, use the following code to get the interpolated z value.
            auto [alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;

            // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
            if (insideTriangle(x, y, t.v))
                set_pixel(Eigen::Vector3f(x, y, 1.0f), color, z_interpolated);
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f &m) {
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f &v) {
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f &p) {
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff) {
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color) {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h) {
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y) {
    return (height - 1 - y) * width + x;
}

/// modified: add depth information（另外，point 完全没必要是 3D 的？而且用 float 会出问题吧。。。）
void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f &color, float depth) {
    // old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    if (depth_buf[ind] >= depth) { // depth smaller, closer
        frame_buf[ind] = color;
        depth_buf[ind] = depth;
    }
}

// clang-format on