#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; // 4 维整数向量，表示这个顶点收到哪些骨骼的影响
layout(location = 6) in vec4 weights;  // 4 维浮点向量，表示这个顶点受到骨骼的影响程度

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES]; // 存储所有骨骼的变换

out vec2 TexCoords;

void main() {
    // 做线性混合蒙皮
    vec4 totalPosition = vec4(0.0f);
    for (int i = 0 ; i < MAX_BONE_INFLUENCE ; i++) {
        if (boneIds[i] == -1)
            continue;
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
   }

    mat4 viewModel = view * model;
    gl_Position =  projection * viewModel * totalPosition;
    TexCoords = tex;
}
