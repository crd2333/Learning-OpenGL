#ifndef __MY_MESH_HPP__
#define __MY_MESH_HPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "my_texture.hpp"

enum AttributeType { // adapt to different shaders
    UNIFORM_COLOR, // all vertices have the same color (a uniform one), 3 floats per vertex
    VERTEX_COLOR,  // each vertex has its own color, 6 floats per vertex
    TEXTURE,       // each vertex has its own texture coordinate, 5 floats per vertex (must give a texture)
    NORMAL_TEXTURE // each vertex has its own normal and texture coordinate, 8 floats per vertex
};

// self define a class for all meshes to do those boring initialization and binding
class MyMesh {
protected:
    GLuint VAO, VBO, EBO = 0;
    std::vector<float> vertices;
    std::vector<int> indices;
    AttributeType attrType;

    void init();         // initialize and bind VAO, VBO
    void init_indices(); // initialize and bind VAO, VBO, EBO for indexed mesh
    void ReadFromObj(const char* path);
public:
    void WriteObj(const char* path); // public func

    // ----- constructors -----
    MyMesh() : attrType(UNIFORM_COLOR) {}  // default constructor
    MyMesh(AttributeType attrType=UNIFORM_COLOR) :
            attrType(attrType) {}
    // if vertices and indices are provided
    MyMesh(std::vector<float> &vertices, AttributeType attrType=UNIFORM_COLOR) :
            vertices(vertices), attrType(attrType) { init(); }  // non-indexed
    MyMesh(std::vector<float> &vertices, std::vector<int> &indices, AttributeType attrType=UNIFORM_COLOR) :
            vertices(vertices), indices(indices), attrType(attrType) { init_indices(); } // indexed
    // if read from .obj file
    MyMesh(const char* path, AttributeType attrType=UNIFORM_COLOR) : attrType(attrType) {
        ReadFromObj(path); init_indices();
    }

    virtual ~MyMesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        if (EBO != 0)
            glDeleteBuffers(1, &EBO);
    }

    int getNumVertices() const { return vertices.size() / getNumFloatsPerVertex(); }
    int getNumIndices() const { return indices.size(); }
    int getNumFloatsPerVertex() const {
        return attrType == UNIFORM_COLOR ? 3 :
              (attrType == VERTEX_COLOR ? 6 :
              (attrType == TEXTURE ? 5 : 8));  // each vertex has 3/6/5/8 floats
    }
    std::vector<float>& getVertices() { return vertices; }
    std::vector<int>& getIndices() { return indices; }

    void draw(Shader &shader, GLenum mode=GL_TRIANGLES) {
        shader.use();
        glBindVertexArray(VAO);
        if (EBO != 0) // if has an indices
            glDrawElements(mode, static_cast<GLuint>(getNumIndices()), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, getNumVertices());
    }

    // for debug
    void printVertices() {
        for (int i = 0; i < (int)vertices.size(); i += getNumFloatsPerVertex()) {
            for (int j = 0; j < getNumFloatsPerVertex(); ++j)
                std::cout << vertices[i + j] << " ";
            std::cout << std::endl;
        }
    }
    // for debug
    void printIndices() {
        for (int i = 0; i < (int)indices.size(); i += 3)
            std::cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << std::endl;
    }
};

// read and write .obj file
// normals are not supported yet
void MyMesh::ReadFromObj(const char* path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Error: failed to open file " << path << std::endl;
        exit(0);
    }

    std::string line;
    int vertex_pos_num = 0;
    int vertex_text_num = 0;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            vertex_pos_num++;
            float p0, p1, p2;
            iss >> p0 >> p1 >> p2;
            vertices.insert(vertices.end(), {p0, p1, p2});
        } else if (type == "vt") {
            vertex_text_num++;
            float p0, p1;
            iss >> p0 >> p1;
            vertices.insert(vertices.begin() + vertex_text_num * getNumFloatsPerVertex() + 3, {p0, p1});
        } else if (type == "f") {
            int p0, p1, p2;
            iss >> p0 >> p1 >> p2;
            indices.insert(indices.end(), {p0-1, p1-1, p2-1}); // 转换为从 0 开始编号
        }
    }
    if (attrType == VERTEX_COLOR) {
        assert(vertex_pos_num == vertex_text_num);
    }
}
void MyMesh::WriteObj(const char* path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "ERROR::WRITE_OBJ::Cannot open file " << path << std::endl;
        return;
    }

    for (int i = 0; i < (int)vertices.size(); i += getNumFloatsPerVertex()) {
        out << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << std::endl;
    }
    if (attrType == TEXTURE) {
        for (int i = 0; i < (int)vertices.size(); i += getNumFloatsPerVertex()) {
            out << "vt " << vertices[i + 3] << " " << vertices[i + 4] << std::endl;
        }
    }
    for (int i = 0; i < (int)indices.size(); i += 3) {
        out << "f " << indices[i] + 1 << "/" << indices[i] + 1 << " "
                    << indices[i + 1] + 1 << "/" << indices[i + 1] + 1 << " "
                    << indices[i + 2] + 1 << "/" << indices[i + 2] + 1 << std::endl;
    }

    out.close();
}

void MyMesh::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    switch (attrType) {
    case UNIFORM_COLOR: // 3 floats per vertex
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        break;
    case VERTEX_COLOR: // 6 floats per vertex, 3 position, 3 color
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        break;
    case TEXTURE: // 5 floats per vertex, 3 position, 2 texture coordinate
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case NORMAL_TEXTURE: // 8 floats per vertex, 3 position, 3 normal, 2 texture coordinate
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        break;
    }
}

void MyMesh::init_indices() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumIndices() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    switch (attrType) {
    case UNIFORM_COLOR:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        break;
    case VERTEX_COLOR:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        break;
    case TEXTURE:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case NORMAL_TEXTURE:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        break;
    }
}

#endif