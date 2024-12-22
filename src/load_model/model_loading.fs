#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_trans1;
uniform vec3 Kd;
uniform vec3 Ka;
uniform vec3 Ks;
uniform vec3 Ke;
uniform bool useTransTexture;

// todo: use normal to do bump mapping and lighting
void main() {
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    float alpha = useTransTexture ? texture(texture_trans1, TexCoords).r : 1.0;
    FragColor = vec4(color, alpha);
}