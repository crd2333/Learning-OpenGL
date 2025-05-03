// #include <chrono>
// #include <iostream>
// #include <opencv2/opencv.hpp>

// std::vector<cv::Point2f> control_points; // 控制点的容器，默认为 4 个

// void mouse_handler(int event, int x, int y, int flags, void* userdata) {
//     if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) { // 如果控制点小于 4 个
//         std::cout << "Left button of the mouse is clicked - position (" << x << ", "
//                   << y << ")" << '\n';
//         control_points.emplace_back(x, y);
//     }
//     // 并没有实现移动控制点的功能
// }

// void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) {
//     auto &p_0 = points[0];
//     auto &p_1 = points[1];
//     auto &p_2 = points[2];
//     auto &p_3 = points[3];

//     for (double t = 0.0; t <= 1.0; t += 0.001) {
//         auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
//                      3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

//         window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
//     }
// }

// cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) {
//     // TODO: Implement de Casteljau's algorithm
//     auto &p_0 = control_points[0];
//     auto &p_1 = control_points[1];
//     auto &p_2 = control_points[2];
//     auto &p_3 = control_points[3];

//     auto q_0 = (1 - t) * p_0 + t * p_1;
//     auto q_1 = (1 - t) * p_1 + t * p_2;
//     auto q_2 = (1 - t) * p_2 + t * p_3;

//     auto r_0 = (1 - t) * q_0 + t * q_1;
//     auto r_1 = (1 - t) * q_1 + t * q_2;

//     return (1 - t) * r_0 + t * r_1;
// }

// void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) {
//     // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
//     // recursive Bezier algorithm.
//     for (double t = 0.0; t <= 1.0; t += 0.001) {
//         auto point = recursive_bezier(control_points, t);
//         window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
//     }
// }

// int main() {
//     cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0)); // 显示窗口
//     cv::cvtColor(window, window, cv::COLOR_BGR2RGB);            // 颜色转换
//     cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);       // 窗口命名

//     cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr); // 把 mouse_handler 函数绑定到窗口

//     int key = -1;
//     while (key != 27) { // 非 esc 键
//         for (auto &point : control_points) { // 绘制控制点
//             cv::circle(window, point, 3, {255, 255, 255}, 3);
//         }

//         if (control_points.size() == 4) {
//             naive_bezier(control_points, window);
//             bezier(control_points, window);

//             cv::imshow("Bezier Curve", window);
//             cv::imwrite("my_bezier_curve.png", window);
//             key = cv::waitKey(0); // 无期限地等待按键，持续显示图像

//             return 0;
//         } else {
//             cv::imshow("Bezier Curve", window);
//             key = cv::waitKey(20); // 等待 20 ms
//         }
//     }

//     return 0;
// }

#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<std::pair<cv::Point2f, double>> control_points; // 控制点的容器，默认为 4 个
int to_be_replaced = -1;

void mouse_handler(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) { // 左键
        if (control_points.size() < 4) { // 如果控制点小于 4 个
            std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                    << y << ")" << '\n';
            control_points.emplace_back(cv::Point2f(x, y), 255);
        } else if (control_points.size() == 4 && to_be_replaced == -1) { // already have 4 points, find one to replace
            // find the nearest point
            double min_dist = 10000.0, threshold = 40.0;
            int idx = -1;
            for (int i = 0; i < control_points.size(); i++) {
                int dist = std::pow(control_points[i].first.x - x, 2) + std::pow(control_points[i].first.y - y, 2);
                if (dist < threshold && dist < min_dist) {
                    min_dist = dist;
                    idx = i;
                }
            }
            if (idx != -1) {
                to_be_replaced = idx;
                control_points[to_be_replaced].second = 128; // decrease the brightness, indicating that it is selected
                std::cout << "Selected point " << to_be_replaced << " : " << control_points[to_be_replaced].first << std::endl;
            }
        } else if (control_points.size() == 4 && to_be_replaced != -1) { // replace the selected point
            control_points[to_be_replaced].first = cv::Point2f(x, y);
            control_points[to_be_replaced].second = 255;
            std::cout << "Point " << to_be_replaced << " updated to:" << control_points[to_be_replaced].first << std::endl;
            to_be_replaced = -1;
        }
    }
    // 并没有实现移动控制点的功能
}

void naive_bezier(const std::vector<std::pair<cv::Point2f, double>> &points, cv::Mat &window) {
    auto &p_0 = points[0].first;
    auto &p_1 = points[1].first;
    auto &p_2 = points[2].first;
    auto &p_3 = points[3].first;

    for (double t = 0.0; t <= 1.0; t += 0.001) {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255; // R
    }
}

cv::Point2f recursive_bezier(const std::vector<std::pair<cv::Point2f, double>> &control_points, float t) {
    // TODO: Implement de Casteljau's algorithm
    auto &p_0 = control_points[0].first;
    auto &p_1 = control_points[1].first;
    auto &p_2 = control_points[2].first;
    auto &p_3 = control_points[3].first;

    auto q_0 = (1 - t) * p_0 + t * p_1;
    auto q_1 = (1 - t) * p_1 + t * p_2;
    auto q_2 = (1 - t) * p_2 + t * p_3;

    auto r_0 = (1 - t) * q_0 + t * q_1;
    auto r_1 = (1 - t) * q_1 + t * q_2;

    return (1 - t) * r_0 + t * r_1;
}

void bezier(const std::vector<std::pair<cv::Point2f, double>> &control_points, cv::Mat &window) {
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.
    for (double t = 0.0; t <= 1.0; t += 0.001) {
        auto point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255; // G
    }
}

int main() {
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0)); // 显示窗口
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);            // 颜色转换
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);       // 窗口命名

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr); // 把 mouse_handler 函数绑定到窗口

    int key = -1;
    while (key != 27) { // 非 esc 键
        window.setTo(cv::Scalar(0)); // clear the window

        for (auto &point : control_points) { // 绘制控制点
            cv::circle(window, point.first, 3, {point.second, point.second, point.second}, 3);
        }

        if (control_points.size() == 4) {

            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(20);

            // return 0;
        } else {
            cv::imshow("Bezier Curve", window);
            key = cv::waitKey(20);
        }
    }

    return 0;
}
