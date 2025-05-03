#include <fstream>
#include "Vector.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include <optional>

inline float deg2rad(const float &deg)
{ return deg * M_PI / 180.0; }

// Compute reflection direction 反射方向，注意算出来可能需要加负号才是真的反射光线，因此用的时候需要和 N 点积判断一下再使用（下面折射也是一样）
Vector3f reflect(const Vector3f &I, const Vector3f &N) {
    return I - 2 * dotProduct(I, N) * N;
}

// [comment]
// Compute refraction direction using Snell's law   // 折射定律，也称为斯涅尔定律(Snell'sLaw)
// We need to handle with care the two possible situations:
//    - When the ray is inside the object
//    - When the ray is outside.
// If the ray is outside, you need to make cosi positive cosi = -N.I
// If the ray is inside, you need to invert the refractive indices and negate the normal N
// [/comment]
Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) {
    float cosi = clamp(-1, 1, dotProduct(I, N)); // 按道理两个单位向量点积本来就应该在 [-1, 1] 之间，可能只是为了保险一点（或者浮点数精度问题？）
    float etai = 1, etat = ior;
    Vector3f n = N;
    if (cosi < 0) cosi = -cosi;
    else { std::swap(etai, etat); n = -N; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi); // 判断是否发生全反射
    return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
}

// [comment]
// Compute Fresnel equation
// \param I is the incident view direction
// \param N is the normal at the intersection point
// \param ior is the material refractive index
// [/comment]
// fresnel（菲涅尔）方程，描述了光线从一种介质射入另一种介质时的反射和折射光线的比例，具体就不详细了解了
float fresnel(const Vector3f &I, const Vector3f &N, const float &ior) {
    float cosi = clamp(-1, 1, dotProduct(I, N));
    float etai = 1, etat = ior;
    if (cosi > 0)   std::swap(etai, etat);
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1)
        return 1;
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}

// [comment]
// Returns true if the ray intersects an object, false otherwise.
// \param orig is the ray origin
// \param dir is the ray direction
// \param objects is the list of objects the scene contains
// \param[out] tNear contains the distance to the cloesest intersected object.
// \param[out] index stores the index of the intersect triangle if the interesected object is a mesh.
// \param[out] uv stores the u and v barycentric coordinates of the intersected point
// \param[out] *hitObject stores the pointer to the intersected object (used to retrieve material information, etc.)
// \param isShadowRay is it a shadow ray. We can return from the function sooner as soon as we have found a hit.
// [/comment]
std::optional<hit_payload> trace(
    const Vector3f &orig, const Vector3f &dir,
    const std::vector<std::unique_ptr<Object> > &objects) {
    float tNear = kInfinity;
    std::optional<hit_payload> payload;
    for (const auto &object : objects) { // 遍历所有物体（可以用 bounding box 加速）
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (object->intersect(orig, dir, tNearK, indexK, uvK) && tNearK < tNear) {
            payload.emplace();
            payload->hit_obj = object.get(); // 从智能指针中获取原始指针
            payload->tNear = tNearK;
            payload->index = indexK;
            payload->uv = uvK; // 重心坐标
            tNear = tNearK;
        }
    }

    return payload;
}

// [comment]
// Implementation of the Whitted-style light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refraction direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refraction depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is diffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
// [/comment]
Vector3f castRay(
    const Vector3f &orig, const Vector3f &dir, const Scene &scene,
    int depth) {
    if (depth > scene.maxDepth)
        return Vector3f(0.0, 0.0, 0.0);

    Vector3f hitColor = scene.backgroundColor;
    if (auto payload = trace(orig, dir, scene.get_objects()); payload) {
        Vector3f hitPoint = orig + dir * payload->tNear;
        Vector3f N;  // normal
        Vector2f st; // st coordinates
        payload->hit_obj->getSurfaceProperties(hitPoint, dir, payload->index, payload->uv, N, st); // 获得法向量 N 和纹理坐标 st
        switch (payload->hit_obj->materialType) { // 根据材质类型进行不同的光线处理
        case REFLECTION_AND_REFRACTION: { // 反射＋折射
            Vector3f reflectionDirection = normalize(reflect(dir, N));
            Vector3f refractionDirection = normalize(refract(dir, N, payload->hit_obj->ior));
            Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                         hitPoint - N * scene.epsilon :
                                         hitPoint + N * scene.epsilon; // 这个微小偏移应该只是为了区分内外光线用的
            Vector3f refractionRayOrig = (dotProduct(refractionDirection, N) < 0) ?
                                         hitPoint - N * scene.epsilon :
                                         hitPoint + N * scene.epsilon;
            Vector3f reflectionColor = castRay(reflectionRayOrig, reflectionDirection, scene, depth + 1); // recursive
            Vector3f refractionColor = castRay(refractionRayOrig, refractionDirection, scene, depth + 1);
            float kr = fresnel(dir, N, payload->hit_obj->ior); // 根据菲涅尔公式加权平均
            hitColor = reflectionColor * kr + refractionColor * (1 - kr);
            break;
        }
        case REFLECTION: { // 仅反射
            float kr = fresnel(dir, N, payload->hit_obj->ior);
            Vector3f reflectionDirection = reflect(dir, N);
            Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                         hitPoint + N * scene.epsilon :
                                         hitPoint - N * scene.epsilon;
            hitColor = castRay(reflectionRayOrig, reflectionDirection, scene, depth + 1) * kr;
            break;
        }
        default: { // DIFFUSE_AND_GLOSSY，漫反射和高光
            // We use the Phong illumation model int the default case. The phong model
            // is composed of a diffuse and a specular reflection component. // 没算 Ambient
            Vector3f lightAmt = 0, specularColor = 0;
            Vector3f shadowPointOrig = (dotProduct(dir, N) < 0) ?
                                       hitPoint + N * scene.epsilon :
                                       hitPoint - N * scene.epsilon;
            // Loop over all lights in the scene and sum their contribution up
            // We also apply the lambert cosine law
            for (auto &light : scene.get_lights()) {
                Vector3f lightDir = light->position - hitPoint;
                // square of the distance between hitPoint and the light
                float lightDistance2 = dotProduct(lightDir, lightDir);
                lightDir = normalize(lightDir);
                float LdotN = std::max(0.f, dotProduct(lightDir, N));
                // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                auto shadow_res = trace(shadowPointOrig, lightDir, scene.get_objects());
                bool inShadow = shadow_res && (shadow_res->tNear * shadow_res->tNear < lightDistance2); // 如果 payload 存在且第一个距离小于光源距离，说明被阻隔

                // diffuse 和 specular 不用除以 lightDistance2 吗？ 除了反而错了？？？
                // 不理解，可能是基于不随距离衰减的假设。但是 bling phone 模型就是要除的啊？？？
                lightAmt += inShadow ? 0 : light->intensity * LdotN;  // 光照强度
                Vector3f reflectionDirection = reflect(-lightDir, N);

                specularColor += powf(std::max(0.f, -dotProduct(reflectionDirection, dir)),
                                      payload->hit_obj->specularExponent) * light->intensity;
            }
            hitColor = lightAmt * payload->hit_obj->evalDiffuseColor(st) * payload->hit_obj->Kd + specularColor * payload->hit_obj->Ks;
            break;
        }
        }
    }

    return hitColor;
}

// The main render function. This where we iterate over all pixels in the image, generate
// primary rays and cast these rays into the scene. The content of the framebuffer is
// saved to a file.
void Renderer::Render(const Scene &scene) {
    std::vector<Vector3f> framebuffer(scene.width * scene.height); // 帧缓存，储存了整个屏幕那么多的像素

    float scale = std::tan(deg2rad(scene.fov * 0.5f));
    float imageAspectRatio = scene.width / (float)scene.height;    // 图像的宽高比，通过 fov 得到的是水平 scale，所以要乘以这个比例

    // Use this variable as the eye position to start your rays.
    Vector3f eye_pos(0);
    int m = 0;
    for (int j = 0; j < scene.height; ++j) {
        for (int i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x;
            float y;
            // TODO: Find the x and y positions of the current pixel to get the direction
            // vector that passes through it.
            // Also, don't forget to multiply both of them with the variable *scale*, and
            // x (horizontal) variable with the *imageAspectRatio*
            x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale; // 先转换到 [-1, 1] 内，然后乘以 scale * imageAspectRatio
            y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;                   // 先转换到 [-1, 1] 内，然后乘以 scale
            Vector3f dir = Vector3f(x, y, -1); // Don't forget to normalize this direction!
            framebuffer[m++] = castRay(eye_pos, normalize(dir), scene, 0); // 每个像素生成一条 path 进行追踪
        }
        UpdateProgress(j / (float)scene.height); // 根据双重循环的外层输出进度条
    }

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (char)(255 * clamp(0, 1, framebuffer[i].x));
        color[1] = (char)(255 * clamp(0, 1, framebuffer[i].y));
        color[2] = (char)(255 * clamp(0, 1, framebuffer[i].z));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
