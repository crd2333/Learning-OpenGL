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
#include "game_frame.h"
#include "my_texture.hpp"
#include "sphere.hpp"
#include "skybox.hpp"
#include "ubo.hpp"

int main() {
    GLFWwindow* window = GameFrame::Create_glfw_Window();
    GameFrame::InitImGui(window);

    // build and compile our shader program
    // sun use the original shader (with a texture)
    Shader shader_sun("src/5.Advanced_Solar_System/resources/sun.vs", "src/5.Advanced_Solar_System/resources/sun.fs");
    // other planets use the shader that deals with lighting (ambient, diffuse, specular), and use a texture as diffuse map
    Shader shader_planet("src/5.Advanced_Solar_System/resources/planet.vs", "src/5.Advanced_Solar_System/resources/planet.fs");
    // earth has specular texture, use a different shader
    Shader shader_earth("src/5.Advanced_Solar_System/resources/planet.vs", "src/5.Advanced_Solar_System/resources/earth.fs");
    // skybox shader
    Shader shader_skybox("src/5.Advanced_Solar_System/resources/skybox.vs", "src/5.Advanced_Solar_System/resources/skybox.fs");

    // define the textures
    // textures come from https://www.solarsystemscope.com/textures/
    Texture2D* sunText = new Texture2D();
    sunText->Generate("src/5.Advanced_Solar_System/resources/textures/sun.jpg");
    Texture2D* earthText = new Texture2D();
    earthText->Generate("src/5.Advanced_Solar_System/resources/textures/earth.jpg");
    Texture2D* marsText = new Texture2D();
    marsText->Generate("src/5.Advanced_Solar_System/resources/textures/mars.jpg");
    Texture2D* moonText = new Texture2D();
    moonText->Generate("src/5.Advanced_Solar_System/resources/textures/moon.jpg");
    Texture2D* earthSpecular = new Texture2D();
    earthSpecular->Generate("src/5.Advanced_Solar_System/resources/textures/earth_specular.jpg");

    // define a base sphere for all stars/planets/satellites
    Sphere sphere(Latitude_resolution, Longitude_resolution);

    // skybox
    Skybox* skybox = new Skybox("src/5.Advanced_Solar_System/resources/textures/skybox");

    // set the fixed, shared properties of shaders
    // sun shader
    shader_sun.use();
    shader_sun.setInt("u_Texture", 0); // set the texture unit
    // planet shader
    shader_planet.use();
    shader_planet.setInt("material.diffuse", 0);
    // earth shader
    shader_earth.use();
    shader_earth.setInt("material.diffuse", 0);
    shader_earth.setInt("material.specular", 1);
    // create and bind the uniform buffer objects
    UBO UBO_Proj_View(2 * sizeof(glm::mat4), 0, "Proj_View");
    UBO_Proj_View.Bind(shader_sun);
    UBO_Proj_View.Bind(shader_planet);
    UBO_Proj_View.Bind(shader_earth);
    UBO UBO_Light(72, 1, "Light"); // size 和 offset 和 learnOpenGL 中介绍的不一样，这里是我自己 debug 出来的（感觉很奇怪。。。）
    UBO_Light.Bind(shader_planet);
    UBO_Light.Bind(shader_earth);

    bool SolarSystemMenu = false;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        GameFrame::RenderLoopPreProcess(window);

        // ImGui windows
        if (mainMenu) {
            ImGui::Begin("Main Menu: left ALT to focus", &mainMenu);
            if (ImGui::Button("Quit")) { // quit button
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloat("Camera Speed", &speedUp, 0.0f, 10.0f);
            ImGui::ColorEdit3("Clear Color", clearColor);
            ImGui::Checkbox("Solar System Menu", &SolarSystemMenu);
            ImGui::End();
        }
        if (mainMenu && SolarSystemMenu) {
            ImGui::Begin("Solar System Menu", &SolarSystemMenu);
            ImGuiSliderFloatWithDefault("Speed Up Ratio", &speedUpRatio, 0.0f, 100.0f, 20.0f);
            ImGuiColorEdit3WithDefault("Light Color", glm::value_ptr(sunColor), glm::vec3(1.0f, 1.0f, 1.0f));
            ImGuiSliderFloat3WithDefault("Light Ambient", glm::value_ptr(sunAmbient), 0.0f, 1.0f, 0.2f);
            ImGuiSliderFloat3WithDefault("Light Diffuse", glm::value_ptr(sunDiffuse), 0.0f, 1.0f, 1.0f);
            ImGuiSliderFloat3WithDefault("Light Specular", glm::value_ptr(sunSpecular), 0.0f, 1.0f, 1.0f);
            ImGuiSliderFloatWithDefault("Light Attenuation Constant", &attenuation_constant, 0.01f, 2.0f, 1.0f);
            ImGuiSliderFloatWithDefault("Light Attenuation Linear", &attenuation_linear, 0.0f, 0.02f, 0.007f);
            ImGuiSliderFloatWithDefault("Light Attenuation Quadratic", &attenuation_quadratic, 0.0f, 0.01f, 0.002f);
            ImGuiSliderFloat3WithDefault("Sun Position", glm::value_ptr(sunPos), -20.0f, 20.0f, 0.0f);
            ImGuiSliderFloatWithDefault("Sun Scale", &sunScale, 0.1f, 10.0f, 2.0f);
            ImGuiSliderFloatWithDefault("Sun Rotation Angle", &sunRotationAngle, 0.0f, 180.0f, 7.25f);
            ImGuiSliderFloatWithDefault("Sun Rotation Speed", &sunRotationSpeed, 0.0f, 10.0f, 360.0f/25.38f/24.0f);
            ImGui::Separator();
            ImGuiSliderFloatWithDefault("Earth Shininess", &earthShininess, 0.0f, 256.0f, 32.0f);
            ImGuiSliderFloatWithDefault("Earth Scale", &earthScale, 0.1f, 10.0f, 0.5f);
            ImGuiSliderFloatWithDefault("Sun Earth Distance", &sunEarthDis, 0.0f, 20.0f, 10.0f);
            ImGuiSliderFloatWithDefault("Earth Rotation Angle", &earthRotationAngle, 0.0f, 180.0f, 23.5f);
            ImGuiSliderFloatWithDefault("Earth Rotation Speed", &earthRotationSpeed, 0.0f, 180.0f, 360.0f/24.0f);
            ImGuiSliderFloatWithDefault("Earth Revolution Speed", &earthRevolutionSpeed, 0.0f, 180.0f, 360.0f/365.26f/24.0f);
            ImGui::Separator();
            ImGuiSliderFloat3WithDefault("Moon Specular", glm::value_ptr(moonSpecular), 0.0f, 1.0f, 0.4f);
            ImGuiSliderFloatWithDefault("Moon Shininess", &moonShininess, 0.0f, 256.0f, 32.0f);
            ImGuiSliderFloatWithDefault("Moon Scale", &moonScale, 0.1f, 10.0f, 0.3f);
            ImGuiSliderFloatWithDefault("Earth Moon Distance", &earthMoonDis, 0.0f, 5.0f, 2.0f);
            ImGuiSliderFloatWithDefault("Moon Rotation Angle", &moonRotationAngle, 0.0f, 180.0f, 1.54f);
            ImGuiSliderFloatWithDefault("Moon Rotation Speed", &moonRotationSpeed, 0.0f, 10.0f, 360.0f/27.32f/24.0f);
            ImGuiSliderFloatWithDefault("Moon Revolution Speed", &moonRevolutionSpeed, 0.0f, 180.0f, 360.0f/27.32f/24.0f);
            ImGui::Separator();
            ImGuiSliderFloat3WithDefault("Mars Specular", glm::value_ptr(marsSpecular), 0.0f, 1.0f, 0.4f);
            ImGuiSliderFloatWithDefault("Mars Shininess", &marsShininess, 0.0f, 256.0f, 32.0f);
            ImGuiSliderFloatWithDefault("Mars Scale", &marsScale, 0.1f, 10.0f, 0.4f);
            ImGuiSliderFloatWithDefault("Sun Mars Distance", &sunMarsDis, 0.0f, 20.0f, 15.0f);
            ImGuiSliderFloatWithDefault("Mars Rotation Angle", &marsRotationAngle, 0.0f, 180.0f, 25.19f);
            ImGuiSliderFloatWithDefault("Mars Rotation Speed", &marsRotationSpeed, 0.0f, 10.0f, 360.0f/24.7f/24.0f);
            ImGuiSliderFloatWithDefault("Mars Revolution Speed", &marsRevolutionSpeed, 0.0f, 180.0f, 60.0f/687.0f/24.0f);
            ImGui::End();
        }

        // set the shared shader properties
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // set projection and view for shader sun, planet and earth
        UBO_Proj_View.setData(0, sizeof(glm::mat4), glm::value_ptr(projection));
        UBO_Proj_View.setData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        // set light's position, ambient, diffuse, specular and attenuation(constant, linear, quadratic) for shader planet and earth
        UBO_Light.setData(0, sizeof(glm::vec3), glm::value_ptr(sunPos));
        UBO_Light.setData(16, sizeof(glm::vec3), glm::value_ptr(sunAmbient));
        UBO_Light.setData(32, sizeof(glm::vec3), glm::value_ptr(sunDiffuse));
        UBO_Light.setData(48, sizeof(glm::vec3), glm::value_ptr(sunSpecular));
        UBO_Light.setData(60, sizeof(float), &attenuation_constant);
        UBO_Light.setData(64, sizeof(float), &attenuation_linear);
        UBO_Light.setData(68, sizeof(float), &attenuation_quadratic);

        float time = glfwGetTime();

        // render the solar system
        // ----------------- sun -----------------
        sunText->Bind(0); // u_Texture
        glm::mat4 sun_model = glm::translate(glm::mat4(1.0f), sunPos);
        // rotate the sun around its axis, first tilt the sun on its axis
        sun_model = glm::rotate(sun_model, glm::radians(sunRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        sun_model = glm::rotate(sun_model, glm::radians(time * sunRotationSpeed * speedUpRatio), glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the sun
        sun_model = glm::scale(sun_model, glm::vec3(sunScale));
        shader_sun.use();
        shader_sun.setMat4("model", sun_model);
        sphere.draw(shader_sun);
        // ----------------- sun -----------------

        // ----------------- earth -----------------
        earthText->Bind(0);     // material.diffuse
        earthSpecular->Bind(1); // material.specular
        glm::mat4 earth_model = glm::mat4(1.0f);
        // revolve around the sun
        float earth_x = - sunEarthDis * sin(glm::radians(time * earthRevolutionSpeed * speedUpRatio));
        float earth_z = - sunEarthDis * cos(glm::radians(time * earthRevolutionSpeed * speedUpRatio));
        earth_model = glm::translate(earth_model, glm::vec3(earth_x, 0.0f, earth_z));
        // rotate the earth around its axis, first tilt the earth on its axis
        earth_model = glm::rotate(earth_model, glm::radians(earthRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        earth_model = glm::rotate(earth_model, glm::radians(time * earthRotationSpeed * speedUpRatio), glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the earth
        earth_model = glm::scale(earth_model , glm::vec3(earthScale)); // scale the earth
        shader_earth.use();
        shader_earth.setMat4("model", earth_model);
        shader_earth.setMat4("normal_mat_m", glm::transpose(glm::inverse(earth_model)));
        shader_planet.setFloat("material.shininess", earthShininess);
        sphere.draw(shader_earth);
        // ----------------- earth -----------------

        // ----------------- moon -----------------
        moonText->Bind(0); // material.diffuse
        glm::mat4 moon_model = glm::mat4(1.0f);
        // revolve around the earth
        float moon_x = earth_x + earthMoonDis * sin(glm::radians(time * moonRevolutionSpeed * speedUpRatio));
        float moon_z = earth_z + earthMoonDis * cos(glm::radians(time * moonRevolutionSpeed * speedUpRatio));
        moon_model = glm::translate(moon_model, glm::vec3(moon_x, 0.0f, moon_z));
        // rotate the moon around its axis, first tilt the moon on its axis
        moon_model = glm::rotate(moon_model, glm::radians(moonRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        moon_model = glm::rotate(moon_model, glm::radians(time * moonRotationSpeed * speedUpRatio), glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the moon
        moon_model = glm::scale(moon_model , glm::vec3(moonScale));
        shader_planet.use();
        shader_planet.setMat4("model", moon_model);
        shader_planet.setMat4("normal_mat_m", glm::transpose(glm::inverse(moon_model)));
        shader_planet.setVec3("material.specular", moonSpecular);
        shader_planet.setFloat("material.shininess", moonShininess);
        sphere.draw(shader_planet);
        // ----------------- moon -----------------

        // ----------------- mars -----------------
        marsText->Bind(0); // material.diffuse
        glm::mat4 mars_model = glm::mat4(1.0f);
        // revolve around the sun
        float mars_x = - sunMarsDis * sin(glm::radians(time * marsRevolutionSpeed * speedUpRatio));
        float mars_z = - sunMarsDis * cos(glm::radians(time * marsRevolutionSpeed * speedUpRatio));
        mars_model = glm::translate(mars_model, glm::vec3(mars_x, 0.0f, mars_z));
        // rotate the mars around its axis, first tilt the mars on its axis
        mars_model = glm::rotate(mars_model, glm::radians(marsRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        mars_model = glm::rotate(mars_model, glm::radians(time * marsRotationSpeed * speedUpRatio), glm::vec3(0.0f, 1.0f, 0.0f));
        // scale the mars
        mars_model = glm::scale(mars_model , glm::vec3(marsScale));
        shader_planet.use();
        shader_planet.setMat4("model", mars_model);
        shader_planet.setMat4("normal_mat_m", glm::transpose(glm::inverse(mars_model)));
        shader_planet.setVec3("material.specular", marsSpecular);
        shader_planet.setFloat("material.shininess", marsShininess);
        sphere.draw(shader_planet);
        // ----------------- mars -----------------

        // render skybox at last
        skybox->draw(view, projection);

        GameFrame::RenderLoopPostProcess(window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}