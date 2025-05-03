#version 330 core

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Color;

uniform vec3 color;
uniform int texture_type = 0;
uniform sampler2D u_Texture;

void main()
{
    if (texture_type == 0)
        FragColor = vec4(color, 1.0);
    else if (texture_type == 1)
        FragColor = vec4(v_Color, 1.0);
    else
        FragColor = texture(u_Texture, v_TexCoord);
}