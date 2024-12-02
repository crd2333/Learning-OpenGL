#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec4 position;
} gs_in[];

out vec2 TexCoords;

uniform float time;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

vec4 explode(vec4 position, vec3 normal) {
    float magnitude = 1.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 GetNormal() {
    vec3 pos0 = vec3(gs_in[0].position);
    vec3 pos1 = vec3(gs_in[1].position);
    vec3 pos2 = vec3(gs_in[2].position);

    vec3 a = pos0 - pos1;
    vec3 b = pos2 - pos1;
    return normalize(cross(b, a));
}

void main() {
    mat4 mvp = projection * view * model;
    vec3 normal = GetNormal();
    // 位置朝法向移动，但纹理坐标不变，构成新的顶点
    gl_Position = mvp * explode(gs_in[0].position, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();
    gl_Position = mvp * explode(gs_in[1].position, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();
    gl_Position = mvp * explode(gs_in[2].position, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}