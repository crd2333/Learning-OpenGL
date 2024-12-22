#version 330 core
out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D texturez;

void main()
{
    if(texture(texturez, texcoord).a < 0.5) // 如果太透明就直接扔掉了，这种情况不会存储在深度缓冲区中
        discard;
    FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}


