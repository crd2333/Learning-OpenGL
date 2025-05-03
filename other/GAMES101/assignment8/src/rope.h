#ifndef ROPE_H
#define ROPE_H

#include "CGL/CGL.h"
#include "mass.h"
#include "spring.h"

using namespace std;

namespace CGL {

class Rope { // Rope 对象，由多个 nodes 组成，其中一些是固定的，每个 node 之间由 spring 连接，每个 node 对应一个质点(mass)
public:
    // 这种构造方法直接指定了每个 mass 的属性和每个 spring 的属性
    Rope(vector<Mass*> &masses, vector<Spring*> &springs) : masses(masses), springs(springs) {}
    // 在 start 和 end 间均匀生成 num_nodes 个 mass（共享质量），将 pinned_nodes 中的设为固定，用 spring 连接（共享弹簧系数）
    Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k,
         vector<int> pinned_nodes);

    void simulateVerlet(float delta_t, Vector2D gravity);
    void simulateEuler(float delta_t, Vector2D gravity);

    vector<Mass*> masses;
    vector<Spring*> springs;
}; // struct Rope
}
#endif /* ROPE_H */
