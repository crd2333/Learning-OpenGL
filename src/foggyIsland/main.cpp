// #define GLEW_STATIC

#include "Water.h"
#include "Skybox.h"
#include "ResourceManager.h"
#include "camera.h"
#include "Terrain.h"
#include "Model.h"
#include "Light.h"
#include "Fog.h"
#include "Framebuffer.h"
#include "Geometry.h"

#include <glad/glad.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

Camera camera(glm::vec3(0.0f, 10.0f, 0.0f));

// settings
const GLuint SCR_WIDTH = 1280;
const GLuint SCR_HEIGHT = 720;
const float near = 0.1f;
const float far = 750.0f;
float heightScale = 0.1f;
glm::vec2 terrainWaterSize = glm::vec2(750.0f); // 水和山的公用大小，山的地势低的时候被水覆盖
float waterHeight = 2.f;
const GLuint NR_TREES = 250;
const GLfloat TREE_SCALE = 2.0f;
const GLfloat HOUSE_SCALE = 0.7f;

//camera data for generating view matrix
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float pitch{ 0.0f }, yaw{ -90.0f };
float fov{ 45.0f };

float deltaTime{ 0.0f };
float UpdateTime{ 0.0f };

// Light
const glm::vec3 lightDir = glm::normalize(glm::vec3(0.43f, 0.76f, -0.33f));//(0.6f, 0.76f, -0.25f)
const glm::vec3 lightColor = glm::vec3(1.f, 1.f, 0.7f);
const GLfloat ambientStrength = 0.08;
const glm::vec3 ambientColor = glm::vec3(0.25f, 0.25f, 1.f);

// Fog
const float fogDensity = 0.0045f;
const glm::vec3 fogColor1 = glm::vec3(0.604f, 0.655f, 0.718f);
const glm::vec3 fogColor2 = glm::vec3(0.631f, 0.651f, 0.698f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool cursorFlag{ false };

int main() {
    //Initiallize GLFW and generate window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Foggy Island", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // tell GLFW to capture our mouse

    // Initialize GLEW to setup the OpenGL Function pointers
    // glewExperimental = GL_TRUE;
    // glewInit();
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glGetError();

    // Set Window
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Display waiting information while program is loading up
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader loadingShader = ResourceManager::LoadShader("src/foggyIsland/shaders/post_processing.vs", "src/foggyIsland/shaders/loading.fs", nullptr, "quad");
    Texture2D loadingTexture = ResourceManager::LoadTexture("src/foggyIsland/resources/textures/LoadingPicture.png", GL_TRUE, "lensstar");
    loadingShader.SetInteger("loadingPicture", 0, GL_TRUE);
    loadingTexture.Bind(0);
    Geometry::DrawPlane(); // 直接在屏幕上画一个四边形
    glfwSwapBuffers(window);

    // Load Models
    Model house("src/foggyIsland/models/house/farmhouse.obj");
    house.calculateBoundingVolume(); // 每个模型抽象成球体计算包围球
    Model tree("src/foggyIsland/models/tree3/laubbaum.obj");
    tree.calculateBoundingVolume();

    // Load Shaders
    Shader shaderSkybox = ResourceManager::LoadShader("src/foggyIsland/shaders/skybox.vs", "src/foggyIsland/shaders/skybox.fs", nullptr, "shaderSkybox");
    Shader shaderTerrain = ResourceManager::LoadShader("src/foggyIsland/shaders/terrain.vs", "src/foggyIsland/shaders/terrain.fs", nullptr, "shaderTerrain");
    Shader shaderHouse = ResourceManager::LoadShader("src/foggyIsland/shaders/house.vs", "src/foggyIsland/shaders/house.fs", nullptr, "shaderHouse");
    Shader SimpleShader = ResourceManager::LoadShader("src/foggyIsland/shaders/simple.vs", "src/foggyIsland/shaders/simple.fs", nullptr, "SimpleShader");
    Shader treeShader = ResourceManager::LoadShader("src/foggyIsland/shaders/tree.vs", "src/foggyIsland/shaders/tree.fs", nullptr, "treeShader");
    Shader treeSimpleShader = ResourceManager::LoadShader("src/foggyIsland/shaders/simple_tree.vs", "src/foggyIsland/shaders/simple_tree.fs", nullptr, "treeSimpleShader");
    Shader sunShader = ResourceManager::LoadShader("src/foggyIsland/shaders/sun.vs", "src/foggyIsland/shaders/sun.fs", nullptr, "quad");
    Shader volumetricShader = ResourceManager::LoadShader("src/foggyIsland/shaders/post_processing.vs", "src/foggyIsland/shaders/volumetric_lighting.fs", nullptr, "quad");
    Shader gaussianBlurShader = ResourceManager::LoadShader("src/foggyIsland/shaders/post_processing.vs", "src/foggyIsland/shaders/gaussian_blur.fs", nullptr, "quad");
    Shader applyPostProcessShader = ResourceManager::LoadShader("src/foggyIsland/shaders/post_processing.vs", "src/foggyIsland/shaders/applyPostProcess.fs", nullptr, "quad");

    // Load Textures
    // 它的 terrain 只用了一种纹理，按我的想法应该要多个纹理，然后根据高度、法向来混合
    Texture2D textureTerrain = ResourceManager::LoadTexture("src/foggyIsland/resources/textures/grass_COLOR.png", GL_FALSE, "textureTerrain");
    Texture2D SunTexture = ResourceManager::LoadTexture("src/foggyIsland/resources/textures/sun.png", GL_TRUE, "lensstar");

    // Configure Texture Samplers
    shaderTerrain.SetInteger("terrain", 0, GL_TRUE);
    shaderTerrain.SetInteger("shadowMap", 1);
    shaderHouse.SetInteger("material.texture_diffuse1", 0, GL_TRUE);
    shaderHouse.SetInteger("material.texture_specular1", 1);
    shaderHouse.SetInteger("material.texture_normal1", 2);
    shaderHouse.SetInteger("lightDepthTexture", 3);
    shaderHouse.SetFloat("material.shininess", 16.0f);
    treeShader.SetInteger("texturez", 0, GL_TRUE);
    treeShader.SetInteger("shadowMap", 3);
    volumetricShader.SetInteger("scene", 0, GL_TRUE);
    gaussianBlurShader.SetInteger("image", 0, GL_TRUE);
    applyPostProcessShader.SetInteger("scene", 0, GL_TRUE); // 后处理的 shader
    applyPostProcessShader.SetInteger("normalScene", 1);

    // Terrain
    Terrain terrain;
    terrain.load(terrainWaterSize, 37.5f, 300.0f, "src/foggyIsland/resources/textures/heightmap_island_low_poly.jpg");
    camera.loadTerrain(&terrain);

    // Water
    Water water(terrainWaterSize, waterHeight);
    water.load("src/foggyIsland/resources/textures/water_dudv_blur.jpg", "src/foggyIsland/resources/textures/water_normal.jpg", 100.f);

    // Trees
    // 使用实例化渲染，一次性渲染多个树
    const GLuint NR_TREES = 250;
    const GLfloat TREE_SCALE = 2.0f;
    srand(2348);
    std::vector<glm::mat4> trees;
    for (GLuint i = 0; i < NR_TREES; i++) { // tree 的世界坐标矩阵，随机生成位置
        GLint x = rand() % (int)terrain.getSize().x - terrain.getSize().x * 0.5f;
        GLint z = rand() % (int)terrain.getSize().y - terrain.getSize().y * 0.5f;
        float y = terrain.getHeight(x, z);
        if (y < water.getHeight() + 0.5f) // 保证树不会长在水面上
            continue;
        glm::mat4 model;
        GLfloat scale = TREE_SCALE + ((rand() % 25) - 7.5) / 10.0f;
        model = glm::translate(model, glm::vec3(x, y - 0.05 * scale, z));
        model = glm::scale(model, glm::vec3(scale));
        trees.push_back(model);
    }

    // Houses
    std::vector<glm::vec3> houseLocs;
    houseLocs.push_back(glm::vec3(-200, terrain.getHeight(-200, 80), 80)); // xyz，y 由地形高度决定，设置 5 个房子
    houseLocs.push_back(glm::vec3(-225, terrain.getHeight(-225, 50), 50));
    houseLocs.push_back(glm::vec3(-226, terrain.getHeight(-226, 135), 135));
    houseLocs.push_back(glm::vec3(-23, terrain.getHeight(-23, 172), 172));
    houseLocs.push_back(glm::vec3(260.0f, terrain.getHeight(260.0f, 15.0f), 15.0f));
    std::vector<glm::mat4> housesModels; // 以及它们的 model 矩阵
    for (GLuint i = 0; i < houseLocs.size(); i++) {
        glm::mat4 model;
        model = glm::translate(model, houseLocs[i]);
        model = glm::scale(model, glm::vec3(HOUSE_SCALE));
        housesModels.push_back(model);
    }

    // Skybox
    Skybox skybox(&shaderSkybox); // 天空盒设置了昼夜变化

    skybox.loadDiurnalSkybox(
              "src/foggyIsland/resources/skybox/right.png",
              "src/foggyIsland/resources/skybox/left.png",
              "src/foggyIsland/resources/skybox/top.png",
              "src/foggyIsland/resources/skybox/bottom.png",
              "src/foggyIsland/resources/skybox/back.png",
              "src/foggyIsland/resources/skybox/front.png"
          );

    skybox.loadNocturnalSkybox(
              "src/foggyIsland/resources/skybox/night/right.png",
              "src/foggyIsland/resources/skybox/night/left.png",
              "src/foggyIsland/resources/skybox/night/top.png",
              "src/foggyIsland/resources/skybox/night/bottom.png",
              "src/foggyIsland/resources/skybox/night/back.png",
              "src/foggyIsland/resources/skybox/night/front.png"
          );

    // Shadow framebuffer
    // 阴影的 FBO，它明明抽象了一个 Framebuffer 类，但是这里还是直接在外面写（感觉封装得不够好）
    GLuint const SHADOW_RESOLUTION = 4096; //8192; // 阴影贴图的分辨率，越大越卡
    GLuint ShadowFBO;
    Texture2D shadowDepth;
    shadowDepth.Image_Format = GL_DEPTH_COMPONENT;
    shadowDepth.Internal_Format = GL_DEPTH_COMPONENT;
    shadowDepth.Filter_Min = GL_LINEAR;
    // Manually set depth attachment to clamp to border value of 1.0 depth; ensures areas that are not visible from light are not in shadow.
    shadowDepth.Wrap_S = GL_CLAMP_TO_BORDER;
    shadowDepth.Wrap_T = GL_CLAMP_TO_BORDER;
    shadowDepth.Generate(SHADOW_RESOLUTION, SHADOW_RESOLUTION, NULL);
    shadowDepth.Bind();
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glGenFramebuffers(1, &ShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth.ID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::SHADOW_FRAMEBUFFER" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float lastTime{ 0.0f }; // 就是我们那边的 lastFrame

    // Set Projection Matrix
    glm::mat4 projection = camera.SetProjectionMatrix((float)SCR_WIDTH, (float)SCR_HEIGHT, near, far); // this remains unchanged for every frame
    shaderTerrain.SetMatrix4("projection", projection, GL_TRUE);
    water.m_shader.SetMatrix4("projection", projection, GL_TRUE);
    shaderHouse.SetMatrix4("projection", projection, GL_TRUE);
    treeShader.SetMatrix4("projection", projection, GL_TRUE);
    sunShader.SetMatrix4("projection", projection, GL_TRUE);

    // Sun
    // setShader 就是设置这些 shader 的跟 sun 和 fog 相关的 uniform 变量，调用一次函数就把它们都设置好
    // 我之前是用的 uniform buffer object 实现这种简便设置，相当于是在 OpenGL 层面，相对复杂一点；它这里是在 class 的层面
    // 它的 uniform 变量用的会更多，不过开销上应该差不太多
    Light sun(lightDir, lightColor, ambientStrength, ambientColor);
    sun.setShader(shaderHouse, "sun", GL_TRUE);
    sun.setShader(shaderTerrain, "sun", GL_TRUE);
    sun.setShader(water.m_shader, "sun", GL_TRUE);
    sun.setShader(treeShader, "sun", GL_TRUE);

    // Fog
    // fog 是 foggyIsland 的重点，我们可以看情况要不要抄
    Fog fog(fogDensity, fogColor1);
    fog.setShader(shaderTerrain, "fog", GL_TRUE);
    fog.setShader(water.m_shader, "fog", GL_TRUE);
    fog.setShader(shaderHouse, "fog", GL_TRUE);
    fog.setShader(treeShader, "fog", GL_TRUE);
    fog.setShader(shaderSkybox, "fog", GL_TRUE);

    // Trees - Instanced array
    GLuint VBO_Trees; // 实例化渲染的 VBO，需要自己像设置 VAO 一样绑定 attributes
    glGenBuffers(1, &VBO_Trees);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Trees);
    glBufferData(GL_ARRAY_BUFFER, trees.size() * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
    for (GLuint i = 0; i < tree.Meshes.size(); i++) {
        glBindVertexArray(tree.Meshes[i].VAO);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
    }

    // Render to Texture
    // FBO，封装得不是很彻底
    // 如果是不做 GODRAYS 的话，这里一个都用不到，直接渲染到默认帧缓冲，然后就没事了
    // 否则，会先渲染到 intermediateFramebuffer，然后再渲染到 normalFramebuffer，最后再渲染到 volumetricFBO
    Framebuffer intermediateFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer normalFramebuffer(SCR_WIDTH, SCR_HEIGHT);
    Framebuffer volumetricFBO(SCR_WIDTH, SCR_HEIGHT);
    Texture2D buffer1, buffer2; // Buffers used for ping-ponging between color attachments for Gaussian blur
    // 这两个纹理 buffer（纹理附件）用作高斯模糊传来传去使用，高斯模糊应该是他要做体积光的时候用的
    buffer1.Internal_Format = GL_RGB16F;
    buffer1.Filter_Min = GL_LINEAR;
    buffer1.Mipmap = GL_FALSE;
    buffer1.Wrap_S = GL_CLAMP_TO_EDGE;
    buffer1.Wrap_T = GL_CLAMP_TO_EDGE;
    buffer1.Generate(SCR_WIDTH, SCR_HEIGHT, NULL);

    buffer2.Internal_Format = GL_RGB16F;
    buffer2.Filter_Min = GL_LINEAR;
    buffer2.Mipmap = GL_FALSE;
    buffer2.Wrap_S = GL_CLAMP_TO_EDGE;
    buffer2.Wrap_T = GL_CLAMP_TO_EDGE;
    buffer2.Generate(SCR_WIDTH, SCR_HEIGHT, NULL);

    volumetricFBO.ColorBuffer.Bind();
    volumetricFBO.ColorBuffer.Wrap_S = GL_CLAMP_TO_EDGE; // Clamp to edge so values do not leak into other sides of texture
    volumetricFBO.ColorBuffer.Wrap_T = GL_CLAMP_TO_EDGE;

    // Render loop 下面进入渲染循环，将会比较复杂
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        UpdateTime += deltaTime;

        if (UpdateTime > 1.f) {
            UpdateTime = 0.f;
            glfwSetWindowTitle(window, ("Foggy Island   FPS : " + std::to_string(1.f / deltaTime)).c_str()); // 帧率
        }

        processInput(window);

        // configure view matrices
        glm::mat4 view = camera.GetViewMatrix();
        camera.CalculateViewFrustum(); // 每次渲染重新计算相机视锥
        glm::mat4 matProjectionView = projection * view; // 把二者合并，着色器内部少一个 uniform 变量和矩阵乘法

        // cull trees that are out of frustum
        std::vector<glm::mat4> treeModels; // 删掉不在视锥体内的树
        for (GLuint i = 0; i < trees.size(); i++) {
            if (tree.isInFrustum(camera, trees[i]))
                treeModels.push_back(trees[i]);
        }
        if (treeModels.size() > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO_Trees);
            glBufferSubData(GL_ARRAY_BUFFER, 0, treeModels.size() * sizeof(glm::mat4), &treeModels[0]); // 更新 VBO（实例化数组的 buffer data）
        }

        #pragma region SHADOW
        // 首先是渲染阴影贴图
        /////////////////////////////////////////////////////////
        ///////////////////  SHADOW PASS  ///////////////////////
        /////////////////////////////////////////////////////////

        glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO); // 绑定阴影 FBO
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightMatrix;
        // 正交投影模拟平行光
        // 设置光源的视锥
        GLfloat orthoWidth = 150.0f;
        glm::mat4 lightProjection = glm::ortho(-orthoWidth, orthoWidth, -orthoWidth, orthoWidth, 25.0f, 350.0f);
        // 光源的 view 矩阵，沿着光的方向走很远，然后看向摄像机的位置（照摄像机附近来显示出阴影）
        glm::mat4 lightView = glm::lookAt(sun.m_direction * 250.f + glm::vec3(camera.Position.x, 0.0f, camera.Position.z), glm::vec3(0.0f) + glm::vec3(camera.Position.x, 0.0f, camera.Position.z), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 biasMatrix = glm::mat4( // 用于将 NDC 空间 [-1,1]^3 转换到 [0, 1]^3 空间
                0.5, 0.0, 0.0, 0.0,
                0.0, 0.5, 0.0, 0.0,
                0.0, 0.0, 0.5, 0.0,
                0.5, 0.5, 0.5, 1.0
            );
        lightMatrix = lightProjection * lightView; // Render texture as full texture, add bias only to final matrix (to reposition vertex to 0.0 - 1.0f space)

        // simple shader 只会存储深度值，连颜色都不存储
        /***********************Terrain*********************/
        SimpleShader.Use();
        SimpleShader.SetMatrix4("lightMatrix", lightMatrix);
        SimpleShader.SetMatrix4("model", glm::mat4());
        terrain.render();

        /***********************Houses*********************/
        for (GLuint i = 0; i < housesModels.size(); i++) {
            SimpleShader.SetMatrix4("model", housesModels[i]);
            house.Draw(SimpleShader);
        }

        /***********************Trees*********************/
        if (treeModels.size() > 0) {
            glDisable(GL_CULL_FACE); // 对 tree，临时禁用背面剔除，没太懂
            treeSimpleShader.Use();
            treeSimpleShader.SetMatrix4("lightMatrix", lightMatrix);
            treeSimpleShader.SetFloat("time", glfwGetTime());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
            for (GLuint i = 0; i < tree.Meshes.size(); i++) {
                glBindVertexArray(tree.Meshes[i].VAO);
                glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
            }
            glEnable(GL_CULL_FACE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        lightMatrix = biasMatrix * lightMatrix; // Add bias to lightMatrix (convert NDC to [0.0, 1.0] interval)
                                                // 上面阴影映射时用 NDC，下面？

        #pragma endregion SHADOW // SHADOW 区域中的所有渲染都会被保存到阴影贴图中（主要是记录那个深度值），用于后续的正常渲染

        #pragma region REFRACTION
        ///////////////////////////////////////////////////////////
        ///////////////////  REFRACTION PASS  /////////////////////
        ///////////////////////////////////////////////////////////

        water.initPassRefraction(); // 会开启裁剪平面，只渲染水下部分

        /**********************Terrain********************/
        // 水下部分只需管 terrain
        shaderTerrain.SetMatrix4("view", view, GL_TRUE);
        shaderTerrain.SetInteger("isRefraction", GL_TRUE);
        shaderTerrain.SetInteger("isReflection", GL_FALSE);
        shaderTerrain.SetFloat("waterHeight", water.getHeight());
        shaderTerrain.SetMatrix4("shadowMat", lightMatrix); // 这个 shadowMat 就是那个把 view space 坐标转换到光源空间的 T 矩阵
        shaderTerrain.SetVector3f("viewPos", camera.Position);
        textureTerrain.Bind(0);
        shadowDepth.Bind(1);
        terrain.render();

        water.terminatePassRefraction();

        #pragma endregion REFRACTION

        #pragma region REFLECTION
        ///////////////////////////////////////////////////////////
        ///////////////////  REFLECTION PASS  /////////////////////
        ///////////////////////////////////////////////////////////

        // now use a imaginary camera on the counter position under watersurface
        glm::mat4 imgView = camera.GetImaginaryViewMatrix(water.getHeight()); // 与现摄像机相对水面对称

        water.initPassReflection(); // 会开启裁剪平面，只渲染水上部分，并且把面的正向设置为顺时针

        /**********************Terrain********************/
        shaderTerrain.SetMatrix4("view", imgView, GL_TRUE);
        shaderTerrain.SetInteger("isRefraction", GL_FALSE);
        shaderTerrain.SetInteger("isReflection", GL_TRUE);
        shaderTerrain.SetFloat("waterHeight", water.getHeight());
        shaderTerrain.SetMatrix4("shadowMat", lightMatrix);
        shaderTerrain.SetVector3f("viewPos", camera.Position);
        textureTerrain.Bind(0);
        shadowDepth.Bind(1);
        terrain.render();

        /**********************Trees********************/
        if (treeModels.size() > 0) {
            glDisable(GL_CULL_FACE);
            treeShader.SetMatrix4("view", imgView, GL_TRUE);
            treeShader.SetVector3f("viewPos", camera.Position);
            treeShader.SetFloat("time", glfwGetTime());
            treeShader.SetMatrix4("shadowMat", lightMatrix);
            shadowDepth.Bind(3);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
            for (GLuint i = 0; i < tree.Meshes.size(); ++i) {
                glBindVertexArray(tree.Meshes[i].VAO);
                glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
            }
            glEnable(GL_CULL_FACE);
        }

        /***********************Skybox*********************/
        skybox.render(imgView, projection, 1.f);

        water.terminatePassReflection();

        #pragma endregion REFLECTION

        // Check if the sun is in our sight. If not, skip GOD RAYS pass and POST PROCESSING pass.
        glm::vec4 position = projection * glm::mat4(glm::mat3(view)) * glm::vec4(sun.m_direction * 250.f, 1.f);
        float zValue = position.z;
        glm::vec2 sunPos = glm::vec2(position.x, position.y);
        sunPos /= position.w;
        sunPos = (sunPos * 0.5f) + 0.5f;
        bool doGodRays = true;
        const float margin = 0.05f;
        if (zValue < 0.f || sunPos.x < 0.f - margin || sunPos.x > 1.f + margin || sunPos.y < 0.f - margin || sunPos.y > 1.f + margin)//the interval should be slightly larger than [0, 1], otherwise when the sun appear from the edges of the screen it looks ��ͻȻ
            doGodRays = false;

        #pragma region GOD_RAYS
        ///////////////////////////////////////////////////////////
        ///////////////////////  GOD RAYS  ////////////////////////
        ///////////////////////////////////////////////////////////
        // 所谓 GOD RAYS 就是光线穿过空气中的尘埃或雾气时产生的光束效果，即体积光相关的东西
        if (doGodRays) { // 如果要做 GOD RAYS，就需要先渲染到 intermediateFramebuffer
            intermediateFramebuffer.BeginRender();

            SimpleShader.Use();
            SimpleShader.SetMatrix4("lightMatrix", matProjectionView);
            // 使用 simple shader 渲染场景，它连颜色都不会有，单纯是用来计算深度值

            /***********************Houses*********************/
            for (GLuint i = 0; i < housesModels.size(); i++) {
                if (house.isInFrustum(camera, housesModels[i])) {
                    SimpleShader.SetMatrix4("model", housesModels[i]);
                    house.Draw(SimpleShader);
                }
            }

            /**********************Terrain********************/
            SimpleShader.SetMatrix4("model", glm::mat4());
            terrain.render();

            /***********************Water*********************/
            glm::mat4 model;
            model = glm::translate(model, glm::vec3(0.0f, water.getHeight(), 0.0f));
            model = glm::scale(model, glm::vec3(water.getSize().x, 1.0f, water.getSize().y));
            SimpleShader.SetMatrix4("model", model);
            glBindVertexArray(water.m_VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            /**********************Trees********************/
            if (treeModels.size() > 0) {
                glDisable(GL_CULL_FACE);
                treeSimpleShader.Use();
                treeSimpleShader.SetMatrix4("lightMatrix", matProjectionView);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
                for (GLuint i = 0; i < tree.Meshes.size(); ++i) {
                    glBindVertexArray(tree.Meshes[i].VAO);
                    glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
                }
                glEnable(GL_CULL_FACE);
            }

            /***********************Sun*********************/
            // 只有在 GOD RAYS 时才会绘制太阳，这是体积光的第一步
            sunShader.Use();
            sunShader.SetMatrix4("view", glm::mat4(glm::mat3(view)));// when player walks forward the sun won't be left behind, so to keep the sun around the player, don't translate
            // 太阳被绘制在沿着光线方向很远的位置（深度非常远）
            sunShader.SetVector3f("sunPos", sun.m_direction * 250.f);// check here when the sun quad is too big // debug
            SunTexture.Bind(0);
            Geometry::DrawPlane(); // 然后画一个平面，但是由纹理它会画成一个白色圆片

            intermediateFramebuffer.EndRender(); // 总之，intermediateFramebuffer 里面就只有一个有颜色的太阳圆片，有颜色的树木，和一堆不会画颜色的只有深度值（深度纹理）的玩意儿
        }
        #pragma endregion GOD_RAYS

        #pragma region NORMAL
        ///////////////////////////////////////////////////////////
        /////////////////////  NORMAL PASS  ///////////////////////
        ///////////////////////////////////////////////////////////

        // 如果之前做了 GOD RAYS，渲染结果先存到 normalFramebuffer，之后还要做后处理；
        // 否则直接渲染到屏幕（默认 framebuffer），之后就没事了
        if (doGodRays)
            normalFramebuffer.BeginRender();
        else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        /**********************Terrain********************/
        shaderTerrain.SetMatrix4("view", view, GL_TRUE);
        shaderTerrain.SetInteger("isRefraction", GL_FALSE);
        shaderTerrain.SetInteger("isReflection", GL_FALSE);
        shaderTerrain.SetMatrix4("shadowMat", lightMatrix);
        shaderTerrain.SetVector3f("viewPos", camera.Position);
        textureTerrain.Bind(0);
        shadowDepth.Bind(1);
        terrain.render();

        /***********************Water*********************/
        water.m_shader.SetMatrix4("view", view, GL_TRUE);
        water.m_shader.SetFloat("time", glfwGetTime() / 10.f);
        water.m_shader.SetVector3f("viewPos", camera.Position);
        water.render();

        /***********************Houses*********************/
        shaderHouse.SetMatrix4("view", view, GL_TRUE);
        shaderHouse.SetVector3f("viewPos", camera.Position);
        for (GLuint i = 0; i < housesModels.size(); i++) {
            if (house.isInFrustum(camera, housesModels[i])) {
                shaderHouse.SetMatrix4("model", housesModels[i]);
                house.Draw(shaderHouse);
            }
        }

        /**********************Trees********************/
        if (treeModels.size() > 0) {
            glDisable(GL_CULL_FACE);
            treeShader.SetMatrix4("view", view, GL_TRUE);
            treeShader.SetVector3f("viewPos", camera.Position);
            treeShader.SetFloat("time", glfwGetTime());
            treeShader.SetMatrix4("shadowMat", lightMatrix);
            shadowDepth.Bind(3);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tree.Meshes[0].textures[0].id);
            for (GLuint i = 0; i < tree.Meshes.size(); ++i) {
                glBindVertexArray(tree.Meshes[i].VAO);
                glDrawElementsInstanced(GL_TRIANGLES, tree.Meshes[i].indices.size(), GL_UNSIGNED_INT, 0, treeModels.size());
            }
            glEnable(GL_CULL_FACE);
        }

        /***********************Skybox*********************/
        skybox.render(view, projection, 1.f);

        if (doGodRays)
            normalFramebuffer.EndRender();
        #pragma endregion NORMAL

        #pragma region POST_PROCESSING // 只有之前做了 GOD RAYS 才会做 POST PROCESSING（后处理）
        ///////////////////////////////////////////////////////////
        ////////////////////  POST PROCESSING  ////////////////////
        ///////////////////////////////////////////////////////////
        if (doGodRays) {
            // 后处理干的事情就是体积光和高斯模糊
            glDisable(GL_DEPTH_TEST);

            // 第二步：体积光，以太阳的圆心为中心做放射状的径向模糊
            volumetricFBO.BeginRender();

            volumetricShader.SetVector2f("sunPos", sunPos, true);

            volumetricShader.Use();
            intermediateFramebuffer.ColorBuffer.Bind(0);
            Geometry::DrawPlane();

            volumetricFBO.EndRender(); // volumetricFBO 里面把 intermediateFramebuffer 的内容做了径向模糊（算是体积光近似？）

            // 第三步：高斯模糊，继续在 volumetricFBO 里面渲染
            // Gaussian blur
            int blur_iterations = 4; // 迭代次数，每次要么做水平模糊要么做垂直模糊
            buffer1 = volumetricFBO.ColorBuffer;
            volumetricFBO.Bind();
            volumetricFBO.UpdateColorBufferTexture(buffer2);
            gaussianBlurShader.Use();
            volumetricFBO.BeginRender();
            for (int i = 0; i < blur_iterations; i++) {
                for (int n = 0; n < 2; n++) {
                    gaussianBlurShader.SetInteger("horizontal", n == 0 ? 1 : 0); // Horizontal or vertical based on n
                    volumetricFBO.UpdateColorBufferTexture(n == 0 ? buffer2 : buffer1); // 在两个 buffer 之间传递（ping-pong）
                    if (n == 0)
                        buffer1.Bind(0);
                    else
                        buffer2.Bind(0);
                    Geometry::DrawPlane();
                }
            }
            volumetricFBO.EndRender();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // 最后，渲染到默认 framebuffer，使用 applyPostProcessShader，把正常渲染的内容和光照效果结合（直接加）
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            applyPostProcessShader.Use();
            volumetricFBO.ColorBuffer.Bind(0);
            normalFramebuffer.ColorBuffer.Bind(1);
            Geometry::DrawPlane();

            glEnable(GL_DEPTH_TEST);
        }

        #pragma endregion POST_PROCESSING

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ResourceManager::Clear();
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //move camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cursorFlag = !cursorFlag;
        if (cursorFlag)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float offsetX = xpos - lastX;
    float offsetY = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(offsetX, offsetY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
