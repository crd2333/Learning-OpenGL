#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_reflection1;

void main() {
    // 反射
    // vec3 I = normalize(Position - cameraPos);
    // vec3 R = reflect(I, normalize(Normal));
    // FragColor = vec4(texture(skybox, R).rgb, 1.0); // 使用 R 光线在立方体贴图（仍叫天空盒，但我们要知道它已经不是天空了）上采样，然后抛弃 alpha 值
    // 折射
    // float ratio = 1.00 / 1.52;
    // vec3 I = normalize(Position - cameraPos);
    // vec3 R = refract(I, normalize(Normal), ratio);
    // FragColor = vec4(texture(skybox, R).rgb, 1.0);
    // 加载模型
    vec3 I = normalize(Position - cameraPos);
    vec3 normal = normalize(Normal);
    vec3 R = reflect(I, normal);

    vec3 reflectMap = vec3(texture(texture_reflection1, TexCoords));
    vec3 reflection = vec3(texture(skybox, R).rgb) * reflectMap;

    float diff = max(normalize(dot(normal, -I)), 0.0f);
    vec3 diffuse = diff * vec3(texture(texture_diffuse1, TexCoords));

    FragColor = vec4(diffuse + reflection, 1.0);
}