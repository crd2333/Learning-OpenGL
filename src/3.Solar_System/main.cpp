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
#include "config.h"
#include "sphere.hpp"
#include "texture.hpp"

// camera
Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

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

    Shader shader("src/3.Solar_System/resources/shader_v.vs",
                  "src/3.Solar_System/resources/shader_f.fs");

    // define the textures
    Texture sunText("src/3.Solar_System/resources/textures/sun.jpg");
    Texture earthText("src/3.Solar_System/resources/textures/earth.jpg");
    Texture marsText("src/3.Solar_System/resources/textures/mars.jpg");
    Texture moonText("src/3.Solar_System/resources/textures/moon.jpg");

    // define a base sphere for all stars/planets/satellites
    Sphere sphere(baseRadius, Latitude_resolution, Longitude_resolution);

    // pass uniform matrices to shader
    glm::mat4 model = glm::mat4(1.0f);
    shader.use();
    shader.setMat4("projection", glm::mat4(1.0f));
    shader.setMat4("view", glm::mat4(1.0f));
    shader.setMat4("model", model);
    shader.setInt("u_Texture", 0);

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

        float time = glfwGetTime();

        // ----------------- sun -----------------
        sphere.setTexture(&sunText);
        glm::mat4 sun_model = glm::mat4(1.0f);
        // rotate the sun around its axis, first tilt the sun on its axis
        sun_model = glm::rotate(sun_model, glm::radians(sunRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        sun_model = glm::rotate(sun_model, glm::radians(time * sunRotationSpeed * speedUpRatio),
                                glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the sun
        sun_model = glm::scale(sun_model, glm::vec3(sunScale));
        shader.setMat4("model", sun_model);
        sphere.draw();

        // and one additional sun because of the requirement
        sun_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 8.0f, 0.0f));
        sun_model = glm::scale(sun_model, glm::vec3(sunScale / 2.0f)); // smaller
        shader.setMat4("model", sun_model);
        sphere.draw();
        // ----------------- sun -----------------

        // ----------------- earth -----------------
        sphere.setTexture(&earthText);
        glm::mat4 earth_model = glm::mat4(1.0f);
        // revolve around the sun
        float earth_x = - sunEarthDis * sin(glm::radians(time * earthRevolutionSpeed * speedUpRatio));
        float earth_z = - sunEarthDis * cos(glm::radians(time * earthRevolutionSpeed * speedUpRatio));
        earth_model = glm::translate(earth_model, glm::vec3(earth_x, 0.0f, earth_z));
        // rotate the earth around its axis, first tilt the earth on its axis
        earth_model = glm::rotate(earth_model, glm::radians(earthRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        earth_model = glm::rotate(earth_model, glm::radians(time * earthRotationSpeed * speedUpRatio),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        // scale the earth
        earth_model = glm::scale(earth_model , glm::vec3(earthScale)); // scale the earth
        shader.setMat4("model", earth_model);
        sphere.draw();
        // ----------------- earth -----------------

        // ----------------- moon -----------------
        sphere.setTexture(&moonText);
        glm::mat4 moon_model = glm::mat4(1.0f);
        // revolve around the earth
        float moon_x = earth_x + earthMoonDis * sin(glm::radians(time * moonRevolutionSpeed * speedUpRatio));
        float moon_z = earth_z + earthMoonDis * cos(glm::radians(time * moonRevolutionSpeed * speedUpRatio));
        moon_model = glm::translate(moon_model, glm::vec3(moon_x, 0.0f, moon_z));
        // rotate the moon around its axis, first tilt the moon on its axis
        moon_model = glm::rotate(moon_model, glm::radians(moonRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        moon_model = glm::rotate(moon_model, glm::radians(time * moonRotationSpeed * speedUpRatio),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the moon
        moon_model = glm::scale(moon_model , glm::vec3(moonScale));
        shader.setMat4("model", moon_model);
        sphere.draw();
        // ----------------- moon -----------------

        // ----------------- mars -----------------
        sphere.setTexture(&marsText);
        glm::mat4 mars_model = glm::mat4(1.0f);
        // revolve around the sun
        float mars_x = - sunMarsDis * sin(glm::radians(time * marsRevolutionSpeed * speedUpRatio));
        float mars_z = - sunMarsDis * cos(glm::radians(time * marsRevolutionSpeed * speedUpRatio));
        mars_model = glm::translate(mars_model, glm::vec3(mars_x, 0.0f, mars_z));
        // rotate the mars around its axis, first tilt the mars on its axis
        mars_model = glm::rotate(mars_model, glm::radians(marsRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        mars_model = glm::rotate(mars_model, glm::radians(time * marsRotationSpeed * speedUpRatio),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the mars
        mars_model = glm::scale(mars_model , glm::vec3(marsScale));
        shader.setMat4("model", mars_model);
        sphere.draw();
        // ----------------- mars -----------------

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}