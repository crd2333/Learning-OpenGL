#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
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
    // specular: from specular map, so earth have its own fragment shader
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.shininess);
    vec3 transformed_specular = (texture(material.specular, TexCoords).rgb + vec3(0.67)) / 1.67; // transform the specular: [0, 1] --> [0.4, 1]
    vec3 specular = specular * spec * transformed_specular;

    // decay with distance
    float distance    = length(position - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    vec3 result = (ambient + diffuse + specular) * attenuation;
    FragColor = vec4(result, 1.0);
}