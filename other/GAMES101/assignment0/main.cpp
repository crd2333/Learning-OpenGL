#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>

int main(){

    // Basic Example of cpp
    std::cout << "Example of cpp \n";
    float a = 1.0, b = 2.0;
    std::cout << a << std::endl;
    std::cout << a/b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0/180.0*acos(-1)) << std::endl;

    // Example of vector
    std::cout << "Example of vector \n";
    // vector definition
    Eigen::Vector3f v(1.0f,2.0f,3.0f);
    Eigen::Vector3f w(1.0f,0.0f,0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << 2.0f * v << std::endl;

    // Example of matrix
    std::cout << "Example of matrix \n";
    // matrix definition
    Eigen::Matrix3f i,j;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output \n";
    std::cout << i << std::endl;
    // matrix add i + j
    // matrix scalar multiply i * 2.0
    // matrix multiply i * j
    // matrix multiply vector i * v

    // homework
    std::cout << "Homework \n";
    Eigen::Vector3f p(2.0f, 1.0f, 1.0f); // point (2, 1) in homogeneous coordinate
    Eigen::Matrix3f r, t;
    r << sqrt(2)/2, -sqrt(2)/2, 0,
         sqrt(2)/2, sqrt(2)/2, 0,
         0, 0, 1; // rotation matrix of 45 degrees
    t << 1, 0, 1,
         0, 1, 2,
         0, 0, 1; // translation matrix of (1, 2)
    Eigen::Vector3f p2 = t * r * p; // apply rotation first and then translation
    std::cout << p2 << std::endl;

    // Another solution by GPT
    std::cout << "Another solution by GPT\n";
    Eigen::Matrix2f rotation;
    rotation << sqrt(2)/2, -sqrt(2)/2,
                sqrt(2)/2, sqrt(2)/2; // rotation matrix of 45 degrees
    Eigen::Vector2f translation;
    translation << 1, 2;  // translation vector of (1, 2)
    Eigen::Transform <float, 2, Eigen::Affine> transform;
    transform.linear() = rotation;
    transform.translation() = translation;
    Eigen::Vector2f p3 = transform * p.head<2>();
    std::cout << p3 << std::endl;

    return 0;
}