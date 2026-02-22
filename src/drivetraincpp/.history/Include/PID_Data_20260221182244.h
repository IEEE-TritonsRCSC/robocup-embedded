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
};

/*

void pid_init(
    PID_TypeDef *pid,
    float maxout, float integral_limit,
    float deadband, float target, float kp, float ki, float kd);
void pid_set_constants(PID_TypeDef *pid, float kp, float ki, float kd);
float pid_calculate(PID_TypeDef *pid, float measure);

*/
