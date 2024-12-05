#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos; // 这里接受到的 FragPos 实际上跟 vertex shader out 出的 FragPos 不是一回事，是已经插值后的结果
                 // 严格来说，vertex shader 输出的是 VertexPos，但名字要保持统一

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 0.5;
    int glossy = 32;

    vec3 viewDir = normalize(viewPos - FragPos);
    // ----- Phong -----
    // vec3 reflectDir = reflect(-lightDir, norm);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), glossy);
    // ----- Phong -----
    // ----- Blinn-Phong -----
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), glossy);
    // ----- Blinn-Phong -----
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}