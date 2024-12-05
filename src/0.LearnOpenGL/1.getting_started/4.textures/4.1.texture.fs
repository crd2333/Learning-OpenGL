#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	// texture(), a built-in function，输出就是纹理的（插值）纹理坐标上的（过滤后的）颜色。
	FragColor = texture(texture1, TexCoord);
}