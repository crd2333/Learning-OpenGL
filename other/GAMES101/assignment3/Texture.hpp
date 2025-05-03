//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture {
private:
    cv::Mat image_data;

public:
    Texture(const std::string &name) {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR); // convert color from RGB to BGR
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v) { // 其实不止是 color，如果是其它贴图，这个函数可以返回法向量、高度等信息
        // added: u, v in [0, 1]
        u = std::min(std::max(u, 0.0f), 1.0f);
        v = std::min(std::max(v, 0.0f), 1.0f);
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
