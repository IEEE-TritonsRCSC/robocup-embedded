#include "PID_Outputs.h"
#include "PID_Gains.h"
#include "stdint.h"

class PID_Data
{
private:
    float target;
    float lastNoneZeroTarget;

    float measure;
    float error;
    float last_error;
    float integral;

    float d_buf[3];
    float error_buf[3];

    float output;

    float maxOutput;
    float integralLimit;
    float deadBand;

    uint32_t thistime;
    uint32_t lasttime;
    uint8_t dtime;

    PID_Gains gains;
    PID_Outputs outputs;

public:
    PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains) : maxOutput(maxOutput), integralLimit(integralLimit), deadBand(deadBand), target(target) {}

    void setTarget(float target);
    void setLastNoneZeroTarget
};

/*

void pid_init(
    PID_TypeDef * pid,
    float maxout,
    float integral_limit,
    float deadband,
    float  target,

    float 	kp,
    float 	ki,
    float 	kd) {

    pid->MaxOutput = maxout;
    pid->IntegralLimit = integral_limit;
    pid->target = target;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->output = 0;
}

void pid_set_constants(PID_TypeDef * pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

float pid_calculate(PID_TypeDef *pid, float measure);

*/
