#include "PID_Gains.h"
#include "helpers.h"

void PID_Gains::setP(float p)
{
    this->kp = p;
}
void PID_Gains::setI(float i)
{
    this->ki = i;
}
void PID_Gains::setD(float d)
{
    this->kd = d;
}

float PID_Gains::getP() { return kp; }
float PID_Gains::getI() { return ki; }
float PID_Gains::getD() { return kd; }

PID_Gains::PID_Gains()
{
    this->kp = 0;
    this->ki = 0;
    this->kd = 0;
}

PID_Gains::PID_Gains(float* gains_array)
{
    this->kp = gains_array[0];
    this->ki = gains_array[1];
    this->kd = gains_array[2];
}
