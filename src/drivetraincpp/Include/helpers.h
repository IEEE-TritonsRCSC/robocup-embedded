#pragma once

#include <stdint.h>
#include "PID_Data.h"

void updateDribblerSpeedFromFlag(int dribble_flag, int16_t* dribble_speed);
void applySafetyTimeoutToTargetSpeeds(int timeout, volatile float *targetSpeeds);
void updateMotorPidLoop(PID_Data *motor_pids, volatile float *targetSpeeds, volatile float *speed_data);
void setupMotors(uint16_t* motorPins);
void turnLEDsOff();
