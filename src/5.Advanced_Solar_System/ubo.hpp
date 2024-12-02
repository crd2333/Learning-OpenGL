#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shader.h"

// note: the size and offset calculation is left to the user
class UBO {
public:
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

    void Bind(const Shader& shader) const {
        GLuint uniformBlockIndex = glGetUniformBlockIndex(shader.ID, blockName.c_str());
        glUniformBlockBinding(shader.ID, uniformBlockIndex, bindingPoint);
    }
private:
    GLuint uboID;
    GLuint bindingPoint;
    std::string blockName;
};
