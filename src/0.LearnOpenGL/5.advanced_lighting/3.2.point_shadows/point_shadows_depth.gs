#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

// 利用 geometry shader 在一次渲染中渲染立方体的六个面
// 把每个点用 shadowMatrices（6 个 T 矩阵）变换到六个光源的视角
void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for (int i = 0; i < 3; ++i) { // for each triangle's vertices
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}