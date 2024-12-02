#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec2 texCoords;
    vec4 position;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vs_out.texCoords = aTexCoords;
    vs_out.position = vec4(aPos, 1.0f); // 注意这里和教程不一样，把原始的顶点位置传递给几何着色器，否则会导致粗看正确但细看不对
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}