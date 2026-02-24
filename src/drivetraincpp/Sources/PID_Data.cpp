#include "PID_Data.h"
#include "helpers.h"

PID_Data::PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains)
    : target(target),
      maxOutput(maxOutput),
      integralLimit(integralLimit),
      deadBand(deadBand),
      gains(gains) {}

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
    if (this->integral < -(this->integralLimit)) {
        this->integral = -integralLimit;
    }
}

// Clamping output -> using direct instead of incremental PID
void PID_Data::clampOutput() {
    if (this->output > this->maxOutput) {
        this->output = this->maxOutput;
    }
    if (this->output < -(this->maxOutput)) {
        this->output = -(this->maxOutput);
    }
}

void PID_Data::updateErrorBuf() {
    this->error_buf[2] = this->error_buf[1];
    this->error_buf[1] = this->error_buf[0];
    this->error_buf[0] = this->target - this->measure;
}

void PID_Data::updateDBuf() {
    this->d_buf[2] = this->d_buf[1];
    this->d_buf[1] = this->d_buf[0];
    const float second_diff_coeff = 2.0f;
    this->d_buf[0] = this->error_buf[0] - second_diff_coeff * this->error_buf[1] + this->error_buf[2];
}

void PID_Data::updateProportionalOutput()
{
    this->outputs.setPout(this->gains.getP() * this->error);
}

void PID_Data::updateIntegralOutput()
{
    this->outputs.setIout(this->gains.getI() * this->integral);
}

void PID_Data::updateDerivativeOutput()
{
    this->outputs.setDout(this->gains.getD() * (this->error - this->lastError));
}

void PID_Data::updateOutputSum()
{
    setOutput(this->outputs.getPout() + this->outputs.getIout() + this->outputs.getDout());
}

void PID_Data::updateProportionalOutputFromErrorDelta()
{
    this->outputs.setPout(this->gains.getP() * (this->error - this->lastError));
}

void PID_Data::updateIntegralOutputFromError()
{
    this->outputs.setIout(this->gains.getI() * this->error);
}

void PID_Data::updateDerivativeOutputFromDBuf()
{
    this->outputs.setDout(this->gains.getD() * this->d_buf[0]);
}

void PID_Data::addOutputSum()
{
    this->output += this->outputs.getOutputSum();
}

void PID_Data::updateErrorFromTargetAndMeasure()
{
    setError(this->target - this->measure);
}

void PID_Data::updateLastError()
{
    setLastError(this->error);
}

void PID_Data::runDirectPidStep()
{
    updateProportionalOutput();
    preventIntegralWindup();
    updateIntegralOutput();
    updateDerivativeOutput();
    updateOutputSum();
    clampOutput();
}

void PID_Data::runIncrementalPidStep()
{
    updateErrorBuf();
    updateProportionalOutputFromErrorDelta();
    updateIntegralOutputFromError();
    updateDBuf();
    updateDerivativeOutputFromDBuf();
    addOutputSum();
    clampOutput();
}

float PID_Data::pidCalculate(float measure) {
    setMeasure(measure);
    updateLastError();
    updateErrorFromTargetAndMeasure();

    runDirectPidStep();
    runIncrementalPidStep();

    return this->output;
}
