/**
 * @file helpers.h
 * @brief Small unit-conversion helpers and robot geometry constants.
 */
#pragma once

#define ROBOT_DIAMETER 0.2 // Robot chassis diameter in meters.
#define WHEEL_DIAMETER 0.06 // Wheel diameter in meters.
#define ROBOT_CIRCUMFERENCE ROBOT_DIAMETER * PI // Robot circumference in meters.
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI // Wheel circumference in meters.
#define ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO ROBOT_CIRCUMFERENCE / WHEEL_CIRCUMFERENCE // Converts robot rotation to wheel travel.

#ifndef PI
   #define PI 3.1415926535897932384626433832795
#endif

// Convert degrees to radians.
float degToRad(const float angle);

// Convert degrees per second to wheel revolutions per second.
float degPerSecondToRPS(const float degPerSecond);
