#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace; // 光源空间的片段位置
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

#define use_PCF true

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    // 转换到 NDC 坐标系，在正交投影下没什么用，但透视投影的 w 分量不为 1
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range，因为我们会把 xy 坐标当做纹理贴图的 uv 坐标
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; // 实际上访问的是深度值
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope) 阴影偏移，根据方向来指定
    // 偏移过大可能会出现 Peter Panning（悬浮）现象，正面剔除技巧可以部分解决这个问题
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    if (!use_PCF) { // check whether current frag pos is in shadow
        shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    } else {        // PCF
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // 查询第 0 级 mipmap 的尺寸，得到一个 texel 的大小
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    // GL_CLAMP_TO_BORDER 环绕方式只对投影视锥内部但是超出深度贴图的部分有效，而对投影视锥外部，z 值超出 1.0，借此剔除
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main() {
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // 前面就是 Bling-Phong 光照模型没什么不同
    // 这里计算一个 0 ~ 1 之间的阴影值（1 表示完全遮挡），并把环境光剔除在外
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}