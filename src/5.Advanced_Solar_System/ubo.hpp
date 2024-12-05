//
// Created by mf on 2024/12/5.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include "shader.h"

// Uniform Buffer Object 用于共享多个着色器之间的 uniform 变量，只需一次更新，所有着色器的变量都会一起更新，避免冗余代码
// note: the calculation pf size and offset is left to the user
class UBO {
public:
    // constructor of UBO, define size of the buffer, binding point and block name
    UBO(GLsizeiptr size, GLuint bindingPoint, const std::string& blockName) : bindingPoint(bindingPoint), blockName(blockName) {
        glGenBuffers(1, &uboID);
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
        glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, uboID, 0, size);
    }
    ~UBO() { glDeleteBuffers(1, &uboID); }

    void setData(GLintptr offset, GLsizeiptr size, const void* data) {
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // decide which shader the UBO is bound to
    void Bind(const Shader& shader) const {
        GLuint uniformBlockIndex = glGetUniformBlockIndex(shader.ID, blockName.c_str());
        glUniformBlockBinding(shader.ID, uniformBlockIndex, bindingPoint);
    }
private:
    GLuint uboID;
    GLuint bindingPoint;
    std::string blockName;
};
