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
    float lastError;
    float integral;

    float d_buf[3];
    float error_buf[3];

    float output;

    float maxOutput;
    float integralLimit;
    float deadBand;

    uint32_t thisTime;
    uint32_t lastTime;
    uint8_t dTtime;

    PID_Gains gains;
    PID_Outputs outputs;

public:
    PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains) : maxOutput(maxOutput), integralLimit(integralLimit), deadBand(deadBand), target(target) {}

    void setTarget(float target);
    void setLastNoneZeroTarget(float lastNoneZeroTarget);
    void setMeasure(float measure);
    void setError(float error);
    void setLastError(float lastError);
    void setIntegral(float integral);
    void setD_buf(float d_buf[3]);
    void setError_buf(float error_buf[3]);
    void setOutput(float output);
    void setMaxOutput(float maxOutput);
    void setIntegralLimit(float integralLimit);
    void setDeadBand(float deadBand);
    void setThisTime(uint32_t thisTime);
    void setLastTime(uint32_t lastTime);
    void setDTime(uint32_t dTime);
    void setGains(PID_Gains gains);
    void setOutputs(PID_Outputs outputs);

    // getters
    float getTarget() const;
    float getLastNoneZeroTarget() const;
    float getMeasure() const;
    float getError() const;
    float getLastError() const;
    float getIntegral() const;
    const float* getD_buf() const;
    const float* getError_buf() const;
    float getOutput() const;
    float getMaxOutput() const;
    float getIntegralLimit() const;
    float getDeadBand() const;
    uint32_t getThisTime() const;
    uint32_t getLastTime() const;
    uint8_t getDTime() const;
    const PID_Gains& getGains() const;
    const PID_Outputs& getOutputs() const;

    void preventIntegralWindup();
    void clampOutput()

        float pidCalculate(float measure);
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
