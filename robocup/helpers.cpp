#include "helpers.h"

// convert units from degrees to radians
float degToRad(const float angle) {
  return angle * PI / 180;
}

// convert units from deg/s to rev/s for turn function
float degPerSecondToRPS(const float degPerSecond) {
  return (degPerSecond * ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO) / 360;
}