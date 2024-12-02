#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// settings, scale of 2/3
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;
const float SCALE = (float)SCR_HEIGHT / SCR_WIDTH;

enum AttributeType {
    UNIFORM_COLOR,
    VERTEX_COLOR,
    TEXTURE
};

// define a class for all patterns
class Pattern {
protected:
    unsigned int VAO, VBO, EBO = 0;
    Shader* shader;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    AttributeType attributeType;

    void init();         // initialize and bind VAO, VBO
    void init_indices(); // initialize and bind VAO, VBO, EBO for indexed pattern
public:
    Pattern() : shader(nullptr), attributeType(UNIFORM_COLOR) {}  // default constructor
    Pattern(Shader* shader, std::vector<float> &vertices,
            AttributeType attributeType=UNIFORM_COLOR) : shader(shader), vertices(vertices),
            attributeType(attributeType) { init(); }  // constructor for non-indexed pattern
    Pattern(Shader* shader, std::vector<float> &vertices, std::vector<unsigned int> &indices,
            AttributeType attributeType=UNIFORM_COLOR) : shader(shader), vertices(vertices), indices(indices), attributeType(attributeType) { init_indices(); } // constructor for indexed pattern

    virtual ~Pattern() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        if (EBO != 0)
            glDeleteBuffers(1, &EBO);
    }

    int getNumIndices() const { return indices.size(); }
    int getNumVertices() const {
        int divider = (attributeType == UNIFORM_COLOR ? 3 :
                      (attributeType == VERTEX_COLOR ? 6 : 5)); // each vertex has 3/6/5 floats
        return vertices.size() / divider;
    }

    void draw(GLenum mode = GL_TRIANGLES) {
        shader->use();
        glBindVertexArray(VAO);
        if (EBO != 0)
            glDrawElements(mode, getNumIndices(), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(mode, 0, getNumVertices());
    }
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

    switch (attributeType) {
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
    switch (attributeType) {
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

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DrawRocket", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader shader1("D:\\documents\\OpenGL\\src\\draw_rocket\\shader_v.vs",
                   "D:\\documents\\OpenGL\\src\\draw_rocket\\shader_f.fs");
    Shader shader2("D:\\documents\\OpenGL\\src\\draw_rocket\\shader_v_color.vs",
                   "D:\\documents\\OpenGL\\src\\draw_rocket\\shader_f.fs");

    // triangle
    std::vector<float> triangle_vertices = {
        0.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
    };
    Pattern triangle(&shader1, triangle_vertices);

    // rectangle, use 2 triangles(4 vertices implemention by index), and use positions to copy
    std::vector<float> rect_vertices = {
        0.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,
    };
    std::vector<unsigned int> rect_indices = {
        0, 1, 2,  // first Triangle
        1, 2, 3   // second Triangle
    };
    Pattern rect(&shader1, rect_vertices, rect_indices);

    // grid lines, divide the whole screen [-1, 1] * [-1, 1] into grids
    int grid_width = 12;
    int grid_height = 8;
    std::vector<float> grid_vertices;
    for (int i = 0; i <= grid_width * 2; i++) {
        if (i == grid_width) { // make x axis green
            grid_vertices.insert(grid_vertices.end(), {-1.0f + 1.0f / grid_width * i, -1.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {0.0f, 1.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {-1.0f + 1.0f / grid_width * i,  1.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {0.0f, 1.0f, 0.0f});
        } else {
            grid_vertices.insert(grid_vertices.end(), {-1.0f + 1.0f / grid_width * i,  1.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 1.0f, 1.0f});
            grid_vertices.insert(grid_vertices.end(), {-1.0f + 1.0f / grid_width * i, -1.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 1.0f, 1.0f});
        }
    }
    for (int i = 0; i <= grid_height * 2; i++) {
        if (i == grid_height) { // make y axis red
            grid_vertices.insert(grid_vertices.end(), {-1.0f, -1.0f + 1.0f / grid_height * i, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 0.0f, 0.0f});
            grid_vertices.insert(grid_vertices.end(), { 1.0f, -1.0f + 1.0f / grid_height * i, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 0.0f, 0.0f});
        } else {
            grid_vertices.insert(grid_vertices.end(), { 1.0f, -1.0f + 1.0f / grid_height * i, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 1.0f, 1.0f});
            grid_vertices.insert(grid_vertices.end(), {-1.0f, -1.0f + 1.0f / grid_height * i, 0.0f});
            grid_vertices.insert(grid_vertices.end(), {1.0f, 1.0f, 1.0f});
        }
    }
    Pattern grid(&shader2, grid_vertices, AttributeType::VERTEX_COLOR);

    // pass uniform matrices to shader
    float width = 1.0 / grid_width;
    float height = 1.0 / grid_height; // each grid size
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f)); // scale to the size of a grid
    shader1.use();
    shader1.setMat4("projection", glm::mat4(1.0f));
    shader1.setMat4("view", glm::mat4(1.0f));
    shader1.setMat4("model", model);
    shader1.setVec3("color", glm::vec3(1.0f, 1.0f, 0.0f));
    shader2.use();
    shader2.setMat4("projection", glm::mat4(1.0f));
    shader2.setMat4("view", glm::mat4(1.0f));
    shader2.setMat4("model", glm::mat4(1.0f));

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // input
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the grid
        shader2.use();
        grid.draw(GL_LINES);

        // render the rectangle
        shader1.use();
        glm::mat4 model_rect = glm::translate(model, glm::vec3(0.0f, 3.0f, 0.0f));
        model_rect = glm::scale(model_rect, glm::vec3(6.0f, 2.0f, 1.0f));
        shader1.setMat4("model", model_rect);
        rect.draw();

        // render the three triangles
        glm::mat4 model_tri1 = glm::translate(model, glm::vec3(1.0f, 5.0f, 0.0f));
        model_tri1 = glm::scale(model_tri1, glm::vec3(2.0f, 2.0f, 1.0f));
        shader1.setMat4("model", model_tri1);
        triangle.draw();
        glm::mat4 model_tri2 = glm::translate(model, glm::vec3(1.0f, 3.0f, 0.0f));
        model_tri2 = glm::rotate(model_tri2, -(float)M_PI_2, glm::vec3(0.0f, 0.0f, 1.0f));
        model_tri2 = glm::scale(model_tri2, glm::vec3(2.0f, 2.0f, 1.0f));
        shader1.setMat4("model", model_tri2);
        triangle.draw();
        glm::mat4 model_tri3 = glm::translate(model, glm::vec3(7.0f, 4.0f, 0.0f));
        model_tri3 = glm::rotate(model_tri3, (float)(M_PI_4 + M_PI_2), glm::vec3(0.0f, 0.0f, 1.0f));
        model_tri3 = glm::scale(model_tri3, glm::vec3(sqrtf(2), sqrtf(2), 1.0f));
        shader1.setMat4("model", model_tri3);
        triangle.draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}