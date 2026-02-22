#include "main.h"
#include "helpers.h"
#define DRIBBLE_SPEED 1500
#define TIMEOUT_DELAY 200

void updateDribblerSpeedFromFlag(int dribble_flag, int16_t* dribble_speed)
{
    if (dribble_flag)
    { // Turns dribbling on/off
        *dribble_speed = DRIBBLE_SPEED;
    }
    else
    {
        *dribble_speed = 0;
    }
}

void applySafetyTimeoutToTargetSpeeds(int timeout, volatile float *targetSpeeds)
{
    if (timeout >= TIMEOUT_DELAY)
    { // Safety timeout when UART disconnects
        for (int i = 0; i < 4; ++i)
        {
            targetSpeeds[i] = 0;
        }
    }
}

void updateMotorPidLoop(PID_Data *motor_pids, volatile float *targetSpeeds, volatile float *speed_data)
{
    for (int i = 0; i < 4; ++i)
    { // PID control loop
        motor_pids[i].setTarget(targetSpeeds[i]);
        motor_pids[i].pidCalculate(speed_data[i]);
    }
}

void setupMotors(uint16_t* motorPins)
{
    // Motor setup
    for (int i = 0; i < 5; i++)
    {
        HAL_GPIO_TogglePin(MOTOR_PORT, motorPins[i]);
    }
}

void turnLEDsOff() {
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, LED_OFF);
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, LED_OFF);
}
