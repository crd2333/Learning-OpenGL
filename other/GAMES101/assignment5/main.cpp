// GAMES101 作业五，框架理解可以参考 https://blog.csdn.net/ycrsw/article/details/124199544
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Light.hpp"
#include "Renderer.hpp"

// In the main function of the program, we create the scene (create objects and lights)
// as well as set the options for the render (image width and height, maximum recursion
// depth, field-of-view, etc.). We then call the render function().
int main()
{
    Scene scene(1280, 960);

    // 圆心 (-1, 0, -12)，半径是 2
    auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2);
    sph1->materialType = DIFFUSE_AND_GLOSSY;
    sph1->diffuseColor = Vector3f(0.6, 0.7, 0.8);  // 漫反射底色为(0.6, 0.7, 0.8)

    // 圆心 (0.5, -0.5, -8)，半径是 1.5
    auto sph2 = std::make_unique<Sphere>(Vector3f(0.5, -0.5, -8), 1.5);
    sph2->ior = 1.5;  // 折射率
    sph2->materialType = REFLECTION_AND_REFRACTION;

    // 将两个球体存入场景容器
    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    // 创建一个地面矩形网格，可以看作两个三角形拼接
    // 四个顶点分别是 (-5, -3, -6), (5, -3, -6), (5, -3, -16), (-5, -3, -16)
    Vector3f verts[4] = {{-5,-3,-6}, {5,-3,-6}, {5,-3,-16}, {-5,-3,-16}};
    uint32_t vertIndex[6] = {0, 1, 3, 1, 2, 3}; // 顶点索引，拼成两个三角形。内部是根据总的 index 每三个 vertIndex 组成一个三角形。不是很清楚为什么采用这种结构而不是直接两重数组 Vector<Vector2i>
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}}; // stCoordinates，纹理坐标，设置在四个角落
    auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, st);
    mesh->materialType = DIFFUSE_AND_GLOSSY;

    scene.Add(std::move(mesh)); // 将地面存入场景容器

    // 创建两个光源，位置分别是 (-20, 70, 20) 和 (30, 50, -12)
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5));
    scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5));

    Renderer r; // 创建渲染器
    r.Render(scene);

    return 0;
}