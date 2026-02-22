#include "PID_Data.h"

PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains) : maxOutput(maxOutput), integralLimit(integralLimit), deadBand(deadBand), target(target) {
    
}

void PID_Data::setTarget(float target) {
    this->target = target;
}
void PID_Data::setLastNoneZeroTarget(float lastNoneZeroTarget) {
    this->lastNoneZeroTarget = lastNoneZeroTarget;
}

void PID_Data::setMeasure(float measure) {
    this->measure = measure;
}

void PID_Data::setError(float error) {
    this->error = error;
}

void PID_Data::setLastError(float last_error) {
    this->last_error = last_error;
}

void PID_Data::setIntegral(float integral) {
    this->integral = integral;
}

void PID_Data::setD_buf(float d_buf[3]) {
    this->d_buf[0] = d_buf[0];
    this->d_buf[1] = d_buf[1];
    this->d_buf[2] = d_buf[2];
}

void PID_Data::setError_buf(float error_buf[3]) {
    this->error_buf[0] = error_buf[0];
    this->error_buf[1] = error_buf[1];
    this->error_buf[2] = error_buf[2];
}

void PID_Data::setOutput(float output) {
    this->output = output;
}

void PID_Data::setMaxOutput(float maxOutput) {
    this->maxOutput = maxOutput;
}

void PID_Data::setIntegralLimit(float integralLimit) {
    this->integralLimit = integralLimit;
}

void PID_Data::setDeadBand(float deadBand) {
    this->deadBand = deadBand;
}

void PID_Data::setThisTime(uint32_t thisTime) {
    this->thisTime = thisTime;
}

void PID_Data::setLastTime(uint32_t lastTime) {
    this->lastTime = lastTime;
}

void PID_Data::setDTime(uint32_t dTime) {
    this->dTtime = static_cast<uint8_t>(dTime);
}

void PID_Data::setGains(PID_Gains gains) {
    this->gains = gains;
}

void PID_Data::setOutputs(PID_Outputs outputs) {
    this->outputs = outputs;
}
