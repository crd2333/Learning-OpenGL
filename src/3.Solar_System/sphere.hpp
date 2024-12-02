#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.hpp"

#define PI 3.14159265359

enum AttributeType { // adapt to different shaders
    UNIFORM_COLOR, // all vertices have the same color (a uniform one), 3 floats per vertex
    VERTEX_COLOR,  // each vertex has its own color, 6 floats per vertex
    TEXTURE        // each vertex has its own texture coordinate, 5 floats per vertex (must give a texture)
};

// self define a class for all patterns to do those boring initialization and binding
class Pattern {
protected:
    unsigned int VAO, VBO, EBO = 0;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    AttributeType attrType;
    Texture* texture;

    void init();         // initialize and bind VAO, VBO
    void init_indices(); // initialize and bind VAO, VBO, EBO for indexed pattern
public:
    // constructors
    Pattern() : attrType(UNIFORM_COLOR), texture(nullptr) {}  // default constructor
    Pattern(AttributeType attrType=UNIFORM_COLOR, Texture* texture=nullptr) :
            attrType(attrType), texture(texture) {}
    // if vertices and indices are provided
    Pattern(std::vector<float> &vertices, AttributeType attrType=UNIFORM_COLOR, Texture* texture=nullptr) :
            vertices(vertices), attrType(attrType), texture(texture) { init(); }  // non-indexed
    Pattern(std::vector<float> &vertices, std::vector<unsigned int> &indices, AttributeType attrType=UNIFORM_COLOR, Texture* texture=nullptr) :
            vertices(vertices), indices(indices), attrType(attrType), texture(texture) { init_indices(); } // indexed

    virtual ~Pattern() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        if (EBO != 0)
            glDeleteBuffers(1, &EBO);
    }

    int getNumIndices() const { return indices.size(); }
    int getNumVertices() const {
        return vertices.size() / getNumFloatsPerVertex();
    }
    int getNumFloatsPerVertex() const {
        return attrType == UNIFORM_COLOR ? 3 :
              (attrType == VERTEX_COLOR ? 6 : 5); // each vertex has 3/6/5 floats
    }

    void draw(GLenum mode=GL_TRIANGLES) {
        if (attrType == TEXTURE) {
            assert(texture != nullptr);
            texture->Bind(0); // bind texture, only support slot 0 yet
        }
        glBindVertexArray(VAO);
        if (EBO != 0) // if has an indices
            glDrawElements(mode, getNumIndices(), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, getNumVertices());
    }

    void setTexture(Texture* texture) { this->texture = texture; }
    void WriteObj(const char* path);
};
void Pattern::init_indices() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumIndices() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    switch (attrType) {
    case UNIFORM_COLOR:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        break;
    case VERTEX_COLOR:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case TEXTURE:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    }
}
void Pattern::init() {
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
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case TEXTURE:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    }
}

void Pattern::WriteObj(const char* path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        // std::cerr << "ERROR::WRITE_OBJ::Cannot open file " << path << std::endl;
        return;
    }
    for (int i = 0; i < vertices.size(); i += getNumFloatsPerVertex()) {
        out << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << std::endl;
    }
    if (attrType == TEXTURE) {
        for (int i = 0; i < vertices.size(); i += getNumFloatsPerVertex()) {
            out << "vt " << vertices[i + 3] << " " << vertices[i + 4] << std::endl;
        }
    }
    for (int i = 0; i < indices.size(); i += 3) {
        out << "f " << indices[i] + 1 << "/" << indices[i] + 1 << " "
                    << indices[i + 1] + 1 << "/" << indices[i + 1] + 1 << " "
                    << indices[i + 2] + 1 << "/" << indices[i + 2] + 1 << std::endl;
    }
    out.close();
}

class Sphere : public Pattern {
public:
    Sphere(Texture* texture=nullptr) : Sphere(0.9f, 30, 60, texture) {} // default constructor, delegate to the next one
    Sphere(const float radius, const unsigned int Longitudes, const unsigned int Latitudes, Texture* texture=nullptr);
    ~Sphere() {}

    float getRadius() const { return radius; }
private:
    float radius = 0;
    void makePoint(float u, float v);
};

// latitude u in [0, 180], longitude v in [0, 360], insert a point to vertices (5 floats)
void Sphere::makePoint(float u, float v) {
    float x = radius * std::sin(u * PI / 180) * std::cos(v * PI / 180);
    float y = radius * std::sin(u * PI / 180) * std::sin(v * PI / 180);
    float z = radius * std::cos(u * PI / 180);
    vertices.insert(vertices.end(), {x, y, z, v / 360, u / 180});
}

Sphere::Sphere(const float radius, const unsigned int Longitudes, const unsigned int Latitudes, Texture* texture) : Pattern(TEXTURE, texture), radius(radius) {
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
            unsigned int offset = getNumVertices() - 4;
            indices.insert(indices.end(),
                {offset, offset + 1, offset + 2});
            indices.insert(indices.end(),
                {offset + 1, offset + 3, offset + 2});
        }
    }
    init_indices(); // with indices
}