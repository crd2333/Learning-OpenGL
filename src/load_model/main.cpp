#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include "Model.h"
#include "MeshVC.h"
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void ChangeFocus(GLFWwindow* window, bool focus);
namespace ImGui { // add to ImGui namespace
    void SliderFloatWithDefault(const char* label, float* v, float v_min, float v_max, float v_default);
    void SliderFloat3WithDefault(const char* label, float* v, float v_min, float v_max, float v_default);
    void ColorEdit3WithDefault(const char* label, float* v, glm::vec3 v_default);
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool imguiFocus = false;
float speedUp = 3.0f;

// window
bool mainMenu = false;
bool ESC_pressed = false;
bool ALT_pressed = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 允许键盘控制
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends 设置渲染器后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // tell GLFW to capture our mouse

    // tell stb/stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // Shader ourShader("src/load_model/vertex_color.vs", "src/load_model/vertex_color.fs");
    Shader ourShader("src/load_model/model_loading.vs", "src/load_model/model_loading.fs");

    // load models
    Model ourModel("src/load_model/model/TreeApple/TreeApple_M.obj");
    // MeshVC bomb("src/load_model/model/bomb/bomb_vc.txt");

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ImGui windows
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (mainMenu) {
            ImGui::Begin("Main Menu: left ALT to focus", &mainMenu);
            if (ImGui::Button("Quit")) { // quit button
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloatWithDefault("Speed Up", &speedUp, 0.1f, 10.0f, 3.0f);
            ImGui::End();
        }

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

void ChangeFocus(GLFWwindow* window, bool flag) {
    if (flag) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // glfw 释放鼠标
        ImGui::GetIO().WantCaptureMouse = true; // 告诉 ImGui 捕获鼠标
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // glfw 捕获鼠标
        ImGui::GetIO().WantCaptureMouse = false; // 告诉 ImGui 释放鼠标
    }
    // 将鼠标位置设置到窗口中心（避免偏转）
    glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    // 移动：WASD + 上下
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

    // 检测 ESC 键的按下事件，用于打开/关闭主菜单
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !ESC_pressed) {
        mainMenu = !mainMenu;
        imguiFocus = mainMenu; // 默认开关菜单时也切换焦点
        ChangeFocus(window, imguiFocus);
        ESC_pressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        ESC_pressed = false;

    // 检测 Left Alt 键的按下事件，用于切换鼠标焦点
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && !ALT_pressed) {
        imguiFocus = !imguiFocus;
        ChangeFocus(window, imguiFocus);
        ALT_pressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
        ALT_pressed = false;
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
void mouse_callback([[maybe_unused]] GLFWwindow* window, double xposIn, double yposIn) {
    if (imguiFocus)
        return;

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
void scroll_callback([[maybe_unused]] GLFWwindow* window, [[maybe_unused]] double xoffset, double yoffset) {
    if (imguiFocus)
        return;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// util imgui functions
namespace ImGui { // add to ImGui namespace
void SliderFloatWithDefault(const char* label, float* v, float v_min, float v_max, float v_default) {
    ImGui::SliderFloat(("##" + std::string(label)).c_str(), v, v_min, v_max); // 注意避免重名
    ImGui::SameLine(); // 在同一行显示
    if (ImGui::Button(label)) // 将 label 作为按钮显示
        *v = v_default;
}
void SliderFloat3WithDefault(const char* label, float* v, float v_min, float v_max, float v_default) {
    ImGui::SliderFloat3(("##" + std::string(label)).c_str(), v, v_min, v_max);
    ImGui::SameLine();
    if (ImGui::Button(label)) {
        v[0] = v_default;
        v[1] = v_default;
        v[2] = v_default;
    }
}
void ColorEdit3WithDefault(const char* label, float* v, glm::vec3 v_default) {
    ImGui::ColorEdit3(("##" + std::string(label)).c_str(), v);
    ImGui::SameLine();
    if (ImGui::Button(label)) {
        v[0] = v_default.r;
        v[1] = v_default.g;
        v[2] = v_default.b;
    }
}
}