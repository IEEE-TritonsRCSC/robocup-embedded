#include "PID_Data.h"

PID_Data::PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains) : maxOutput(maxOutput), integralLimit(integralLimit), deadBand(deadBand), target(target) {
    this->gains = gains;
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

void PID_Data::setLastError(float lastError) {
    this->lastError = lastError;
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

float PID_Data::getTarget() const {
    return target;
}

float PID_Data::getLastNoneZeroTarget() const {
    return lastNoneZeroTarget;
}

float PID_Data::getMeasure() const {
    return measure;
}

float PID_Data::getError() const {
    return error;
}

float PID_Data::getLastError() const {
    return lastError;
}

float PID_Data::getIntegral() const {
    return integral;
}

const float* PID_Data::getD_buf() const {
    return d_buf;
}

const float* PID_Data::getError_buf() const {
    return error_buf;
}

float PID_Data::getOutput() const {
    return output;
}

float PID_Data::getMaxOutput() const {
    return maxOutput;
}

float PID_Data::getIntegralLimit() const {
    return integralLimit;
}

float PID_Data::getDeadBand() const {
    return deadBand;
}

uint32_t PID_Data::getThisTime() const {
    return thisTime;
}

uint32_t PID_Data::getLastTime() const {
    return lastTime;
}

uint8_t PID_Data::getDTime() const {
    return dTtime;
}

const PID_Gains& PID_Data::getGains() const {
    return gains;
}

const PID_Outputs& PID_Data::getOutputs() const {
    return outputs;
}

void PID_Data::preventIntegralWindup() {
    this->integral += this->error;

    if (this->integral > this->integralLimit) {
        this->integral = this->integralLimit;
    }
    if (this->integral < -(pid->integralLimit))
}

float PID_Data::pidCalculate(float measure) {
    setMeasure(measure);
    setLastError(this->error);
    setError(this->target - this->measure);

    this->outputs.setPout(this->gains.getP() * this->error);
}
/*

float pid_calculate(PID_TypeDef* pid, float measure)
{
    // Prevent integral windup
    if(pid->integral > pid->IntegralLimit)
    {
        pid->integral = pid->IntegralLimit;
    }
    if(pid->integral < -(pid->IntegralLimit))
    {
        pid->integral = -pid->IntegralLimit;
    }
    pid->iout = pid->ki * pid->integral;

    pid->dout =  pid->kd * (pid->error - pid->lastError);

    pid->output = pid->pout + pid->iout + pid->dout;

    //Clamping output -> using direct instead of incremental PID
    if(pid->output>pid->MaxOutput)
    {
        pid->output = pid->MaxOutput;
    }
    if(pid->output < -(pid->MaxOutput))
    {
        pid->output = -(pid->MaxOutput);
    }



    pid->error_buf[2] = pid->error_buf[1];
    pid->error_buf[1] = pid->error_buf[0];
    pid->error_buf[0] = pid->target - pid->measure;
    pid->pout = pid->kp * (pid->error - pid->lasterror);
    pid->iout = pid->ki * pid->error;
    pid->d_buf[2] = pid->d_buf[1];
    pid->d_buf[1] = pid->d_buf[0];
    pid->d_buf[0] = (pid->error_buf[0] - 2.0f * pid->error_buf[1] + pid->error_buf[2]);
    pid->dout = pid->kd * pid->d_buf[0];
    pid->output += pid->pout + pid->iout + pid->dout;
    if(pid->output>pid->MaxOutput)
    {
        pid->output = pid->MaxOutput;
    }
    if(pid->output < -(pid->MaxOutput))
    {
        pid->output = -(pid->MaxOutput);
    }



    return pid->output;
}


*/
