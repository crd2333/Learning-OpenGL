#version 330 core

out vec4 FragColor;

struct Material {
    // use diffuse as ambient
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

in vec3 FragPos;
in vec3 Normal;    // normalized in vertex shader
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
layout (std140) uniform Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

void main() {
    vec3 lightDir = normalize(position - FragPos);

    // ambient
    vec3 ambient = ambient * texture(material.diffuse, TexCoords).rgb;
    // diffuse
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diffuse * diff * texture(material.diffuse, TexCoords).rgb;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = specular * spec * material.specular;

    // decay with distance
    float distance    = length(position - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    vec3 result = (ambient + diffuse + specular) * attenuation;
    FragColor = vec4(result, 1.0);
}