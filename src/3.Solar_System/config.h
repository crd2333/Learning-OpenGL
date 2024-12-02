#pragma once

// window config
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;
const float SCALE = (float)SCR_HEIGHT / SCR_WIDTH;

// treat glfwGetTime() as hours, and use this to speed up globally
const float speedUpRatio = 100.0f;

// Sphere
const float baseRadius = 1.0f;
const unsigned int Latitude_resolution = 30;
const unsigned int Longitude_resolution = 60;

// Sun
const float sunScale = 2.0f;
const float sunRotationAngle = 7.25f; // tilt angle of the sun
const float sunRotationSpeed = 360.0f/25.38f/24.0f; // rotation around its axis, degrees per hour (average, because it's gas)

// Earth
const float earthScale = 0.5f;
const float sunEarthDis = 10.0f;
const float earthRotationAngle = 23.5f; // tilt angle of the earth
const float earthRotationSpeed = 360.0f/24.0f;           // rotation around its axis, degrees per hour
const float earthRevolutionSpeed = 360.0f/365.26f/24.0f; // revolution around the sun, degrees per hour

// Moon
const float moonScale = 0.3f;
const float earthMoonDis = 2.0f;
const float moonRotationAngle = 1.54f;  // tilt angle of the moon
const float moonRotationSpeed = 360.0f/27.32f/24.0f;   // rotation around its axis, degrees per hour
const float moonRevolutionSpeed = 360.0f/27.32f/24.0f; // revolution around the earth, degrees per hour

// Mars
const float marsScale = 0.4f;
const float sunMarsDis = 15.0f;
const float marsRotationAngle = 25.19f; // tilt angle of the mars
const float marsRotationSpeed = 360.0f/24.7f/24.0f;    // rotation around its axis, degrees per hour
const float marsRevolutionSpeed = 360.0f/687.0f/24.0f; // revolution around the sun, degrees per hour
