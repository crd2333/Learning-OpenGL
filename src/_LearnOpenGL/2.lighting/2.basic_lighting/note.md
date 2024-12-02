- 评论里有这样的说法
  - 法向量矩阵的做法，以及那篇博客，我不太赞同。因为直接取mat4的左上角矩阵块mat3，便直接丢弃了透视数据，博客里的说法也过于武断、不够严谨。我觉得不必取左上角矩阵块mat3，而应该采用整个矩阵 mat4。这篇博客我觉得更科学、严谨：[https://www.songho.ca/opengl/gl_normaltransform.html](https://www.songho.ca/opengl/gl_normaltransform.html#translation)
  - 意思就是说，可以用 mat4，但是法向量要补全为 vec4，然后结果再转换为 vec3，这样既保留了透视数据，又得到了正确的法向量
    ```glsl
    uniform mat4 normal_matrix;
    int main() {
        Normal = vec3(normal_matrix * vec4(aNormal, 1.0));
        ...
    }
    ```
- 关于为什么逆+转置即可得到正确的法向矩阵
    ![](https://uploads.disquscdn.com/images/5666918ed573bb4810bd7af5200be6227d143c43c354c780c1591c5ef1487153.png)
- 另外，glsl 里用 `transpose(inverse(model))` 对每个顶点都算一遍过于慢了，可以在 CPU 算好再传给 uniform
- Phone -> Bling Phone
  ```glsl
  //specular(Blinn-Phong Shading)
  float specularStrength = 0.5;
  int glossy = 32;
  vec3 viewDirection = normalize(viewPos - FragPos);
  vec3 halfDirection = normalize(lightDirection + viewDirection);
  vec3 specular = specularStrength * lightColor * pow(max(dot(normal, halfDirection), 0.0), glossy);
  ```
- 关于 “在观察空间进行光照而不是在世界空间”，或许会更快？
  - 好像没差多少，先不管了