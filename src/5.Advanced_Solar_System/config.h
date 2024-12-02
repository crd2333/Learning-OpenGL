#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "camera.h"

// window config
#define SCR_WIDTH 1500
#define SCR_HEIGHT 1000
#define SCALE ((float)SCR_HEIGHT / SCR_WIDTH)

// camera config
#define CAMERA_POS glm::vec3(0.0f, 5.0f, 20.0f)
#define CAMERA_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_YAW -90.0f
#define CAMERA_PITCH 0.0f

#endif