#include "helpers.h"

// Convert degrees to radians.
float degToRad(const float angle) {
  return angle * PI / 180;
}

// Convert angular speed from degrees per second into wheel revolutions per second.
float degPerSecondToRPS(const float degPerSecond) {
  return (degPerSecond * ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO) / 360;
}
