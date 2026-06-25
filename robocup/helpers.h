#pragma once

#define ROBOT_DIAMETER 0.2 // in meters
#define WHEEL_DIAMETER 0.06 // in meters
#define ROBOT_CIRCUMFERENCE ROBOT_DIAMETER * PI // in meters
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI // in meters
#define ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO ROBOT_CIRCUMFERENCE / WHEEL_CIRCUMFERENCE // in meters per meter

#ifndef PI
   #define PI 3.1415926535897932384626433832795
#endif

// convert units from degrees to radians
float degToRad(const float angle);

// convert units from deg/s to rev/s for turn function
float degPerSecondToRPS(const float degPerSecond);