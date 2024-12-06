#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

// 之前把 depth.fs 置空，让 OpenGL 写深度贴图
// 这里我们自己计算线性距离相关的深度值，使之后的阴影计算更直观
void main() {
    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}