#ifndef __BEZIER_HPP__
#define __BEZIER_HPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "my_mesh.hpp"

#define DEG 4 // fixed at 4 times 4 yet

class Bezier : public MyMesh {
public:
    Bezier(GLfloat* points) : Bezier(points, 5) {}
    Bezier(GLfloat* points, int resolution) : MyMesh(UNIFORM_COLOR), resolution(resolution) {
        // transform GLfloat array to glm::vec3 array
        for (int i = 0; i < DEG * DEG; i++) {
            control_points[i] = glm::vec3(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
        }
        for (int i = 0; i < resolution; i++) {
            for (int j = 0; j < resolution; j++) {
                float u = (float)i / (resolution - 1);
                float v = (float)j / (resolution - 1);
                make_point(u, v);
            }
        }
        for (int i = 0; i < resolution - 1; i++) {
            for (int j = 0; j < resolution - 1; j++) {
                int index = i * resolution + j;
                indices.insert(indices.end(), {index, index + 1, index + resolution});
                indices.insert(indices.end(), {index + 1, index + resolution + 1, index + resolution});
            }
        }
        init_indices();
        // also initialize VAO for control points
        glGenVertexArrays(1, &VAO_cp);
        glGenBuffers(1, &VBO_cp);
        glBindVertexArray(VAO_cp);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_cp);
        glBufferData(GL_ARRAY_BUFFER, DEG * DEG * 3 * sizeof(float), control_points, GL_STATIC_DRAW);
        assert(attrType == UNIFORM_COLOR); // only uniform color is supported now
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
    void draw_control_points(Shader &shader, GLfloat point_size=10.0f, glm::vec3 color=glm::vec3(1.0f,0.0f,0.0f)) {
        shader.use();
        shader.setInt("texture_type", attrType);
        glBindVertexArray(VAO_cp);
        glPointSize(point_size); // 设置点的大小为 10.0
        shader.setVec3("color", color);
        glDrawArrays(GL_POINTS, 0, DEG * DEG);
    }

private:
    unsigned int VAO_cp, VBO_cp;
    const int resolution;
    glm::vec3 control_points[DEG * DEG];
    float bernstein(float t, int index) {
        switch (index) {
            case 0: return (1 - t) * (1 - t) * (1 - t);
            case 1: return 3 * t * (1 - t) * (1 - t);
            case 2: return 3 * t * t * (1 - t);
            case 3: return t * t * t;
        }
        return 0.0f;
    }
    void make_point(float u, float v) {
        glm::vec3 point(0.0f);
        for (int i = 0; i < DEG; i++) {
            for (int j = 0; j < DEG; j++) {
                point += control_points[i * DEG + j] * bernstein(u, i) * bernstein(v, j);
            }
        }
        vertices.insert(vertices.end(), {point.x, point.y, point.z});
    }
};

#endif