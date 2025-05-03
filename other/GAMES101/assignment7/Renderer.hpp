//
// Created by goksu on 2/25/20.
//
#include "Scene.hpp"

#pragma once
struct hit_payload {
    float tNear;
    uint32_t index;
    Vector2f uv;
    Object* hit_obj;
};

class Renderer {
public:
    void Render(const Scene &scene);

private:
    // change the spp value to change sample ammount
    int spp = 2; // spp: sample per pixel
};
