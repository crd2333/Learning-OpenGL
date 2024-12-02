#ifndef __MY_MESH_HPP__
#define __MY_MESH_HPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "my_texture.hpp"
#include "subdivision.hpp"

enum AttributeType { // adapt to different shaders
    UNIFORM_COLOR, // all vertices have the same color (a uniform one), 3 floats per vertex
    VERTEX_COLOR,  // each vertex has its own color, 6 floats per vertex
    TEXTURE        // each vertex has its own texture coordinate, 5 floats per vertex (must give a texture)
};

// self define a class for all meshes to do those boring initialization and binding
class MyMesh {
protected:
    unsigned int VAO, VBO, EBO = 0;
    std::vector<float> vertices;
    std::vector<int> indices;
    AttributeType attrType;
    MyTexture* texture;

    void init();         // initialize and bind VAO, VBO
    void init_indices(); // initialize and bind VAO, VBO, EBO for indexed mesh
    void ReadFromObj(const char* path);
public:
    void WriteObj(const char* path); // public func
    void subdivide(int times=1) {   // use loop subdivision to refine the mesh
        loopSubdivision(vertices, indices, times);
        init_indices(); // reinitialize the VAO, VBO, EBO
    }

    // ----- constructors -----
    MyMesh() : attrType(UNIFORM_COLOR), texture(nullptr) {}  // default constructor
    MyMesh(AttributeType attrType=UNIFORM_COLOR, MyTexture* texture=nullptr) :
            attrType(attrType), texture(texture) {}
    // if vertices and indices are provided
    MyMesh(std::vector<float> &vertices, AttributeType attrType=UNIFORM_COLOR, MyTexture* texture=nullptr) :
            vertices(vertices), attrType(attrType), texture(texture) { init(); }  // non-indexed
    MyMesh(std::vector<float> &vertices, std::vector<int> &indices, AttributeType attrType=UNIFORM_COLOR, MyTexture* texture=nullptr) :
            vertices(vertices), indices(indices), attrType(attrType), texture(texture) { init_indices(); } // indexed
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

    int getNumVertices() const {
        return vertices.size() / getNumFloatsPerVertex();
    }
    int getNumIndices() const { return indices.size(); }
    int getNumFloatsPerVertex() const {
        return attrType == UNIFORM_COLOR ? 3 :
              (attrType == VERTEX_COLOR ? 6 : 5); // each vertex has 3/6/5 floats
    }
    std::vector<float>& getVertices() { return vertices; }
    std::vector<int>& getIndices() { return indices; }

    void draw(Shader &shader, GLenum mode=GL_TRIANGLES) {
        shader.use();
        shader.setInt("texture_type", attrType);
        if (attrType == TEXTURE) {
            assert(texture != nullptr);
            texture->Bind(0); // bind texture, only support slot 0 yet
        }
        glBindVertexArray(VAO);
        if (EBO != 0) // if has an indices
            glDrawElements(mode, static_cast<unsigned int>(getNumIndices()), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, getNumVertices());
    }

    void setTexture(MyTexture* texture) { this->texture = texture; }

    // for debug
    void printVertices() {
        for (int i = 0; i < vertices.size(); i += getNumFloatsPerVertex()) {
            for (int j = 0; j < getNumFloatsPerVertex(); ++j) {
                std::cout << vertices[i + j] << " ";
            }
            std::cout << std::endl;
        }
    }
    void printIndices() {
        for (int i = 0; i < indices.size(); i += 3) {
            std::cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << std::endl;
        }
    }
};

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

// output the mesh to a .obj file
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
    }
}

#endif