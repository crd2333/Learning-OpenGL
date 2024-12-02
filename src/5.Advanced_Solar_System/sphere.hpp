#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "my_mesh.hpp"

#define PI 3.14159265359

class Sphere : public MyMesh {
public:
    Sphere() : Sphere(30, 60) {} // default constructor, delegate to the next one
    Sphere(const unsigned int Longitudes, const unsigned int Latitudes);
    ~Sphere() {}
private:
    void makePoint(float u, float v);
};

// latitude u in [0, 180], longitude v in [0, 360], insert a point to vertices (5 floats)
void Sphere::makePoint(float u, float v) {
    float x = 1.0f * std::sin(u * PI / 180) * std::cos(v * PI / 180);
    float y = 1.0f * std::sin(u * PI / 180) * std::sin(v * PI / 180);
    float z = 1.0f * std::cos(u * PI / 180);
    vertices.insert(vertices.end(), {x, y, z, v / 360, u / 180});
}

// 对于球形这种特殊物体，其 normal 与 position 一致，不需要额外的 normal 数据，这里直接用 TEXTURE 类型
Sphere::Sphere(const unsigned int Longitudes, const unsigned int Latitudes) : MyMesh(TEXTURE) {
    const float lat_step = 180.0f / Latitudes;
    const float lon_step = 360.0f / Longitudes;
    for (unsigned int lat = 0; lat < Latitudes; lat++){      // latitudinal semi-circles u
        for (unsigned int lon = 0; lon < Longitudes; lon++){ // longitudinal circles v
            // 4 points at (u, v), (u, v+1), (u+1, v), (u+1, v+1)
            makePoint(lat * lat_step, lon * lon_step);
            makePoint(lat * lat_step, (lon + 1) * lon_step);
            makePoint((lat + 1) * lat_step, lon * lon_step);
            makePoint((lat + 1) * lat_step, (lon + 1) * lon_step);
            // 2 triangles, (0, 1, 2) and (1, 3, 2)
            int offset = getNumVertices() - 4;
            indices.insert(indices.end(),
                {offset, offset + 1, offset + 2});
            indices.insert(indices.end(),
                {offset + 1, offset + 3, offset + 2});
        }
    }
    init_indices(); // with indices
}

/* config for the solar system */
// treat glfwGetTime() as hours, and use this to speed up globally
float speedUpRatio = 20.0f;

// Sphere
const unsigned int Latitude_resolution = 30;
const unsigned int Longitude_resolution = 60;

// Sun
// light properties: color, pos, ambient, diffuse, specular and attenuation(constant, linear, quadratic)
glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f); // light properties: color, pos, ambient, diffuse, specular
glm::vec3 sunAmbient = glm::vec3(0.2f);
glm::vec3 sunDiffuse = glm::vec3(1.0f);
glm::vec3 sunSpecular = glm::vec3(1.0f);
glm::vec3 sunPos = glm::vec3(0.0f, 0.0f, 0.0f);
float attenuation_constant = 1.0f;    // default attenuation from https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
float attenuation_linear = 0.007f;
float attenuation_quadratic = 0.002f;
// shpere properties
float sunScale = 2.0f;
float sunRotationAngle = 7.25f; // tilt angle of the sun
float sunRotationSpeed = 360.0f/25.38f/24.0f; // rotation around its axis, degrees per hour (average, because it's gas)

// Earth
// material properties: ambient, diffuse, specular, shininess, the former 3 are from the texture
float earthShininess = 32.0f;
// shpere properties
float earthScale = 0.5f;
float sunEarthDis = 10.0f;
float earthRotationAngle = 23.5f; // tilt angle of the earth
float earthRotationSpeed = 360.0f/24.0f;           // rotation around its axis, degrees per hour
float earthRevolutionSpeed = 360.0f/365.26f/24.0f; // revolution around the sun, degrees per hour

// Moon
// material properties: ambient, diffuse, specular, shininess, the former 2 are from the texture
glm::vec3 moonSpecular = glm::vec3(0.4f);
float moonShininess = 32.0f;
// shpere properties
float moonScale = 0.3f;
float earthMoonDis = 2.0f;
float moonRotationAngle = 1.54f;  // tilt angle of the moon
float moonRotationSpeed = 360.0f/27.32f/24.0f;   // rotation around its axis, degrees per hour
float moonRevolutionSpeed = 360.0f/27.32f/24.0f; // revolution around the earth, degrees per hour

// Mars
// material properties: ambient, diffuse, specular, shininess, the former 2 are from the texture
glm::vec3 marsSpecular = glm::vec3(0.4f);
float marsShininess = 32.0f;
// shpere properties
float marsScale = 0.4f;
float sunMarsDis = 15.0f;
float marsRotationAngle = 25.19f; // tilt angle of the mars
float marsRotationSpeed = 360.0f/24.7f/24.0f;    // rotation around its axis, degrees per hour
float marsRevolutionSpeed = 360.0f/687.0f/24.0f; // revolution around the sun, degrees per hour
