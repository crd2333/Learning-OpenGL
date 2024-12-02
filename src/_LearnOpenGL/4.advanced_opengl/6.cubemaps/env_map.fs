#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main() {
    // 反射
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0); // 使用 R 光线在立方体贴图（仍叫天空盒，但我们要知道它已经不是天空了）上采样，然后抛弃 alpha 值
    // 折射
    // float ratio = 1.00 / 1.52;
    // vec3 I = normalize(Position - cameraPos);
    // vec3 R = refract(I, normalize(Normal), ratio);
    // FragColor = vec4(texture(skybox, R).rgb, 1.0);
}