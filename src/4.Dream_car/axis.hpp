#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "my_mesh.hpp"
#include "shader.h"

class Axis {
public:
    Axis();
    ~Axis();
    void draw(Shader& shader);

private:
    GLuint VAO, VBO;
    void setupAxis();
};

Axis::Axis() {
    setupAxis();
}

Axis::~Axis() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Axis::setupAxis() {
    GLfloat axisVertices[] = {
        // X轴 - 红色
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 起点
        3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 终点
        // Y轴 - 绿色
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 起点
        0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 终点
        // Z轴 - 蓝色
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 起点
        0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f  // 终点
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 颜色属性
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Axis::draw(Shader& shader) {
    shader.use();
    shader.setInt("texture_type", VERTEX_COLOR);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}