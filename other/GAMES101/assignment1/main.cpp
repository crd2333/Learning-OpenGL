#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    Eigen::Matrix4f translate;
    translate << cos(rotation_angle), -sin(rotation_angle), 0, 0,
                 sin(rotation_angle), cos(rotation_angle), 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1;
    model = translate * model;

    return model;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    /// TODO: 绕任意过原点的轴
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
    Eigen::Matrix3f rotation3;
    Eigen::Matrix3f N;
    N << 0, -axis.z(), axis.y(),
         axis.z(), 0, -axis.x(),
         -axis.y(), axis.x(), 0;
    rotation3 = cos(angle) * Eigen::Matrix3f::Identity() + (1 - cos(angle)) * axis * axis.transpose() + sin(angle) * N;
    rotation.block(0, 0, 3, 3) = rotation3;
    return rotation;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
    // Students will implement this function

    Eigen::Matrix4f projection;

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    float top = abs(zNear) * tan(eye_fov / 2);
    float down = -top;
    float right = top * aspect_ratio;
    float left = -right;
    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();
    projection << zNear, 0, 0, 0,
                    0, zNear, 0, 0,
                    0, 0, zNear + zFar, -zNear * zFar,
                    0, 0, 1, 0;
    ortho << 2 / (right - left), 0, 0, -(right + left) / (right - left),
             0, 2 / (top - down), 0, -(top + down) / (top - down),
             0, 0, 2 / (zNear - zFar), -(zNear + zFar) / (zNear - zFar),
             0, 0, 0, 1;
    projection = ortho * projection;

    return projection;
}

int main(int argc, const char** argv) {
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4)
            filename = std::string(argv[3]);
        else
            return 0;
    }

    rst::rasterizer r(700, 700); // 700 * 700 的屏幕分辨率

    Eigen::Vector3f eye_pos = {0, 0, 5}; // 相机位置

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}}; // 三角形的三个顶点（硬编码）

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}}; // 三角形的三个顶点的索引

    // 从 pos 和 ind 中加载顶点和索引
    // 一个是 0，一个是 1（？
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // 如果是命令行模式，直接生成图片
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    // 否则，显示窗口，27 为 ESC 键
    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        // std::cout << "Debug: " << r.get_id() << " ";
        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a')
            angle += 10;
        else if (key == 'd')
            angle -= 10;
    }

    return 0;
}
