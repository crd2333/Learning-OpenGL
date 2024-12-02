#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;
// 使用顶点属性时，顶点着色器的每次运行都会让 GLSL 获取新一组适用于当前顶点的属性
// 而当我们将顶点属性定义为一个实例化数组时，顶点着色器就只需要对每个实例，而不是每个顶点，更新顶点属性的内容了
// 也就是说，这里的 aOffset 只会在每个实例中改变，而不是每个顶点

out vec3 fColor;

void main() {
    vec2 pos = aPos * (gl_InstanceID / 100.0);
    fColor = aColor;
    gl_Position = vec4(pos + aOffset, 0.0, 1.0);
}