#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;
vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // 左上
    vec2( 0.0f,    offset), // 正上
    vec2( offset,  offset), // 右上
    vec2(-offset,  0.0f),   // 左
    vec2( 0.0f,    0.0f),   // 中
    vec2( offset,  0.0f),   // 右
    vec2(-offset, -offset), // 左下
    vec2( 0.0f,   -offset), // 正下
    vec2( offset, -offset)  // 右下
);


void main() {
    // ----- initial version -----
    // FragColor = texture(screenTexture, TexCoords);
    // ----- initial version (with alpha removed) -----
    // vec3 col = texture(screenTexture, TexCoords).rgb;
    // FragColor = vec4(col, 1.0);
    // ----- inverted phase -----
    // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
    // ----- grayscale -----
    // FragColor = texture(screenTexture, TexCoords);
    // float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // FragColor = vec4(average, average, average, 1.0); // 灰度图，RGB 值相同，且在 0 ~ 255 之间
    // ----- grayscale (weighted) -----
    // FragColor = texture(screenTexture, TexCoords);
    // float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    // FragColor = vec4(average, average, average, 1.0);
    // ----- kernel -----
    // 锐化核 sharpen kernel
    // float kernel[9] = float[](
    //     -1, -1, -1,
    //     -1,  9, -1,
    //     -1, -1, -1
    // );
    // 模糊核 blur kernel
    // float kernel[9] = float[](
    //     1.0 / 16, 2.0 / 16, 1.0 / 16,
    //     2.0 / 16, 4.0 / 16, 2.0 / 16,
    //     1.0 / 16, 2.0 / 16, 1.0 / 16
    // );
    // 边缘检测核 edge detection kernel
    float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );
    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(col, 1.0);
}