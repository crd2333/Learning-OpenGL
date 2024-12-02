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
#include "camera.h"
#include "bezier.hpp"
#include "my_texture.hpp"
#include "axis.hpp"

const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;
const float SCALE = (float)SCR_HEIGHT / SCR_WIDTH;

// camera
Camera camera(glm::vec3(-5.0f, 2.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0f, 0.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

#define DEG 4
#define FLOATS_PER_VERTEX (DEG * DEG * 3)
GLfloat carVertices[] = {
    // back 车尾
    0,0,0, 0,0,1, 0,0,2, 0,0,3,
    0,1,0, 0,1,1, 0,1,2, 0,1,3,
    0,2,0, 0,2,1, 0,2,2, 0,2,3,
    0,3,0, 0,3,1, 0,3,2, 0,3,3,
    // front 车头
    7,0,0, 7,0,1, 7,0,2, 7,0,3,
    7,0.33,0, 7,0.33,1, 7,0.33,2, 7,0.33,3,
    7,0.67,0, 7,0.67,1, 7,0.67,2, 7,0.67,3,
    7,1,0, 7,1,1, 7,1,2, 7,1,3,
    // roof 顶板
    0,3,0, 0,3,1, 0,3,2, 0,3,3,
    1.33,3,0, 1.33,3,1, 1.33,3,2, 1.33,3,3,
    2.67,3,0, 2.67,3,1, 2.67,3,2, 2.67,3,3,
    4,3,0, 4,3,1, 4,3,2, 4,3,3,
    // floor 底板
    0,0,0, 0,0,1, 0,0,2, 0,0,3,
    2.33,0,0, 2.33,0,1, 2.33,0,2, 2.33,0,3,
    4.67,0,0, 4.67,0,1, 4.67,0,2, 4.67,0,3,
    7,0,0, 7,0,1, 7,0,2, 7,0,3,
    // front_window 前窗
    4,3,0, 4,3,1, 4,3,2, 4,3,3,
    4.33,2.67,0, 4.33,2.67,1, 4.33,2.67,2, 4.33,2.67,3,
    4.67,2.33,0, 4.67,2.33,1, 4.67,2.33,2, 4.67,2.33,3,
    5,2,0, 5,2,1, 5,2,2, 5,2,3,
    // bonnet 前盖
    5,2,0, 5,2,1, 5,2,2, 5,2,3,
    5.67,1.67,0, 5.67,1.67,1, 5.67,1.67,2, 5.67,1.67,3,
    6.33,1.33,0, 6.33,1.33,1, 6.33,1.33,2, 6.33,1.33,3,
    7,1,0, 7,1,1, 7,1,2, 7,1,3,
    // left_back_door 左后门
    0,0,0, 1.33,0,0, 2.67,0,0, 4,0,0,
    0,1,0, 1.33,1,0, 2.67,1,0, 4,1,0,
    0,2,0, 1.33,2,0, 2.67,2,0, 4,2,0,
    0,3,0, 1.33,3,0, 2.67,3,0, 4,3,0,
    // left_front_door 左前门
    4,0,0, 4.33,0,0, 4.67,0,0, 5,0,0,
    4,1,0, 4.33,0.89,0, 4.67,0.78,0, 5,0.67,0,
    4,2,0, 4.33,1.78,0, 4.67,1.56,0, 5,1.33,0,
    4,3,0, 4.33,2.67,0, 4.67,2.33,0, 5,2,0,
    // left_head 左车头
    5,0,0, 5.67,0,0, 6.33,0,0, 7,0,0,
    5,0.67,0, 5.67,0.56,0, 6.33,0.44,0, 7,0.33,0,
    5,1.33,0, 5.67,1.11,0, 6.33,0.89,0, 7,0.67,0,
    5,2,0, 5.67,1.67,0, 6.33,1.33,0, 7,1,0,
    // right_back_door 右后门
    0,0,3, 1.33,0,3, 2.67,0,3, 4,0,3,
    0,1,3, 1.33,1,3, 2.67,1,3, 4,1,3,
    0,2,3, 1.33,2,3, 2.67,2,3, 4,2,3,
    0,3,3, 1.33,3,3, 2.67,3,3, 4,3,3,
    // right_front_door 右前门
    4,0,3, 4.33,0,3, 4.67,0,3, 5,0,3,
    4,1,3, 4.33,0.89,3, 4.67,0.78,3, 5,0.67,3,
    4,2,3, 4.33,1.78,3, 4.67,1.56,3, 5,1.33,3,
    4,3,3, 4.33,2.67,3, 4.67,2.33,3, 5,2,3,
    // right_head 右车头
    5,0,3, 5.67,0,3, 6.33,0,3, 7,0,3,
    5,0.67,3, 5.67,0.56,3, 6.33,0.44,3, 7,0.33,3,
    5,1.33,3, 5.67,1.11,3, 6.33,0.89,3, 7,0.67,3,
    5,2,3, 5.67,1.67,3, 6.33,1.33,3, 7,1,3,
};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    int speedUp = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? 2 : 1; // press left-control to speed up
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime * speedUp);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime * speedUp);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * speedUp);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * speedUp);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
        && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)   // shift + space
        camera.ProcessKeyboard(DOWN, deltaTime / 2 * speedUp);
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // only space
        camera.ProcessKeyboard(UP, deltaTime / 2  * speedUp);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar_System", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // tell GLFW to capture our mouse
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);  // configure global opengl state

    std::string dirPath = "src/4.Dream_car";

    // build and compile our shader program
    Shader shader((dirPath + "/resources/VertShader.glsl").c_str(), (dirPath + "/resources/FragShader.glsl").c_str());

    Bezier back(carVertices, 50);
    Bezier front(carVertices + FLOATS_PER_VERTEX, 50);
    Bezier roof(carVertices + FLOATS_PER_VERTEX * 2, 50);
    Bezier floor(carVertices + FLOATS_PER_VERTEX * 3, 50);
    Bezier front_window(carVertices + FLOATS_PER_VERTEX * 4, 50);
    Bezier bonnet(carVertices + FLOATS_PER_VERTEX * 5, 50);
    Bezier left_back_door(carVertices + FLOATS_PER_VERTEX * 6, 50);
    Bezier left_front_door(carVertices + FLOATS_PER_VERTEX * 7, 50);
    Bezier left_head(carVertices + FLOATS_PER_VERTEX * 8, 50);
    Bezier right_back_door(carVertices + FLOATS_PER_VERTEX * 9, 50);
    Bezier right_front_door(carVertices + FLOATS_PER_VERTEX * 10, 50);
    Bezier right_head(carVertices + FLOATS_PER_VERTEX * 11, 50);

    MyMesh tire((dirPath + "/resources/Cube.obj").c_str());
    tire.subdivide(2);

    // pass uniform matrices to shader
    glm::mat4 model = glm::mat4(1.0f);
    shader.use();
    shader.setMat4("projection", glm::mat4(1.0f));
    shader.setMat4("view", glm::mat4(1.0f));
    shader.setMat4("model", model);
    shader.setInt("u_Texture", 0);

    // Axis axis;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // render the axis
        // axis.draw(shader);

        // render the bezier surface
        shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);
        back.draw(shader);
        front.draw(shader);
        roof.draw(shader);
        floor.draw(shader);
        front_window.draw(shader);
        bonnet.draw(shader);
        left_back_door.draw(shader);
        left_front_door.draw(shader);
        left_head.draw(shader);
        right_back_door.draw(shader);
        right_front_door.draw(shader);
        right_head.draw(shader);

        shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
        // 缩放轮胎，z方向变窄
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.2f));
        // 移动轮胎
        glm::mat4 left_front = glm::translate(glm::mat4(1.0f), glm::vec3(4.7f, -0.2f, 0.0f)) * scale;
        glm::mat4 left_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, -0.2f, 0.0f)) * scale;
        glm::mat4 right_front = glm::translate(glm::mat4(1.0f), glm::vec3(4.7f, -0.2f, 3.0f)) * scale;
        glm::mat4 right_back = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, -0.2f, 3.0f)) * scale;
        shader.setMat4("model", left_front);
        tire.draw(shader, GL_LINE_STRIP);
        shader.setMat4("model", left_back);
        tire.draw(shader, GL_LINE_STRIP);
        shader.setMat4("model", right_front);
        tire.draw(shader, GL_LINE_STRIP);
        shader.setMat4("model", right_back);
        tire.draw(shader, GL_LINE_STRIP);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}