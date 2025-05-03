#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (std140) uniform Proj_View {
    mat4 projection;
    mat4 view;
};
uniform mat4 model;
uniform mat4 normal_mat_m;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    // for sphere objects, normals are just normolized positions (and since positions are already normalized, they are equal)
    Normal = normalize(vec3(normal_mat_m * vec4(aPos, 1.0)));
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}