#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings, scale of 2/3
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;
const float SCALE = (float)SCR_HEIGHT / SCR_WIDTH;

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DrawFlag", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader ourShader("D:\\documents\\OpenGL\\src\\draw_flag\\flag.vs",
                     "D:\\documents\\OpenGL\\src\\draw_flag\\flag.fs");

    /*
    the two shaders are below:
    -------------------------------------------------------------
    #version 330 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(aPos, 1.0f);
    }
    -------------------------------------------------------------
    #version 330 core
    out vec4 FragColor;

    uniform vec3 color;

    void main()
    {
        FragColor = vec4(color, 1.0f);
    }
    */

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // triangle
    glm::vec3 triangle[] = {
        glm::vec3(-1.0f, -1.0f,  0.0f),
        glm::vec3(-1.0f,  1.0f,  0.0f),
        glm::vec3(sqrt(3) * SCALE - 1.0f,  0.0f,  0.0f),
    };

    // rectangle, use 2 triangles(4 vertices implemention by index), and use positions to copy
    glm::vec3 rectangle[] = {
        glm::vec3(-1.0f, -0.2f,  0.0f),
        glm::vec3( 1.0f, -0.2f,  0.0f),
        glm::vec3(-1.0f,  0.2f,  0.0f),
        glm::vec3( 1.0f,  0.2f,  0.0f),
    };
    unsigned int rect_indices[] = {
        0, 1, 2,  // first Triangle
        1, 2, 3   // second Triangle
    };
    glm::vec3 rect_positions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 0.0f,  0.8f,  0.0f),
        glm::vec3( 0.0f, -0.8f,  0.0f)
    };

    // 5-pointed star
    glm::vec3 star[10];
    float theta = 2.0f * M_PI / 20.0f;     // start from 18°
    float increment = 2.0f * M_PI / 10.0f; // 36°
    float r_out   = 1.0/3;
    float r_inner = r_out * sinf(theta) / cosf(increment); // ri sin(54°) = ro sin(18°)
    for (int i = 0; i < 10; ++i, theta += increment) {
        float radius = (i % 2 == 0 ? r_out : r_inner);
        // in x-axis, muiltply 2/3 for scale
        star[i] = glm::vec3(radius * cosf(theta) * SCALE, radius * sinf(theta), 0.0f);
    }
    unsigned int star_indices[] = {
        1, 3, 5, // inner circle
        1, 5, 7,
        1, 7, 9,
        0, 1, 9, // outer circle
        1, 2, 3,
        3, 4, 5,
        5, 6, 7,
        7, 8, 9,
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    unsigned int VAOs[3], VBOs[3], EBOs[2];
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);
    glGenBuffers(2, EBOs);

    // bind the Vertex Array Object, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(star), star, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(star_indices), star_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // pass uniform matrices to shader
    ourShader.use();
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // input
        processInput(window);

        // render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use(); // activate shader

        // render the rectangles
        for (int i = 0; i < 3; i++) {
            ourShader.setMat4("model", glm::translate(glm::mat4(1.0f), rect_positions[i]));
            ourShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
            glBindVertexArray(VAOs[1]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // render the triangle
        ourShader.setMat4("model", glm::mat4(1.0f));
        ourShader.setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // render the 5-pointed star
        ourShader.setMat4("model", glm::translate(model, glm::vec3(sqrt(3)*2/9-1.0f, 0.0f, 0.0f)));
        ourShader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
        glBindVertexArray(VAOs[2]);
        glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(3, VAOs);
    glDeleteBuffers(3, VBOs);
    glDeleteBuffers(2, EBOs);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
