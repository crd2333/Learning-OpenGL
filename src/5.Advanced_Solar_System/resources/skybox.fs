#version 330 core
out vec4 FragColor;

in vec3 TexCoords; // 三维纹理坐标

uniform samplerCube skybox;

void main() {
    float attenuation = 0.7;
    FragColor = texture(skybox, TexCoords) * attenuation;
}