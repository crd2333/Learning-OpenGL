#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "my_texture.hpp"
#include "shader.h"

// TODO: 昼夜切换的天空盒（两个 cube_maps 之间随时间插值？）

class Skybox {
public:
    Skybox(std::string path) {
        // initialize and bind VAO, VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // initialize cube map
        if (path.back() != '/') path += '/';
        std::vector<std::string> faces = {
            path + "right.jpg",
            path + "left.jpg",
            path + "top.jpg",
            path + "bottom.jpg",
            path + "front.jpg",
            path + "back.jpg",
        };
        cube_map = new CUBE_MAP();
        cube_map->Generate(faces);
        // initialize shader
        shader = new Shader(skybox_vs, skybox_fs);
        shader->use();
        shader->setInt("skybox", 0);
    }
    ~Skybox() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        delete cube_map;
        delete shader;
    }
    void draw(const glm::mat4 &view, const glm::mat4 &projection) {
        cube_map->Bind(0);
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix
        glDepthFunc(GL_LEQUAL); // 深度缓冲将会填上 1.0 值，第一次绘制完后之后将再也不会通过，因此要改为小于或等于
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);   // set back to default
    }
private:
    GLuint VAO, VBO;
    CUBE_MAP* cube_map;
    Shader* shader;
    // 天空盒就只是一个立方体，直接硬编码在 code 里
    GLfloat skybox_vertices[3*6*6] = {
        // 天空盒直接把顶点坐标作为纹理坐标，故只用三个 float 表示一个顶点
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    // 直接将天空盒的 shader 硬编码在 code 中（使用原始字符串字面量）
    std::string skybox_vs = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            out vec3 TexCoords;
            uniform mat4 projection;
            uniform mat4 view;
            void main() {
                TexCoords = aPos;
                vec4 pos = projection * view * vec4(aPos, 1.0);
                gl_Position = pos.xyww; // 透视除法是在顶点着色器运行之后执行，这样取使得深度值永远为 1.0（即最远）
            }
        )";
    std::string skybox_fs = R"(
            #version 330 core
            out vec4 FragColor;
            in vec3 TexCoords; // 三维纹理坐标
            uniform samplerCube skybox;
            void main() {
                float attenuation = 0.7; // 找到的纹理有点小亮，调低一手（应该用 uniform 改合理一点）
                FragColor = texture(skybox, TexCoords) * attenuation;
            }
        )";
};
