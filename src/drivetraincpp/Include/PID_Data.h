#pragma once

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
    PID_Data(float maxOutput, float integralLimit, float deadBand, float target, PID_Gains gains);

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

    /**
     * Accumulate integral term and clamp to integral limits to prevent windup.
     */
    void preventIntegralWindup();
    /**
     * Clamp the overall output to the configured max output bounds.
     */
    void clampOutput();
    /**
     * Shift and update the error history buffer from target and measure.
     */
    void updateErrorBuf();
    /**
     * Shift and update the derivative buffer using the error history.
     */
    void updateDBuf();
    /**
     * Compute and store proportional output from current error.
     */
    void updateProportionalOutput();
    /**
     * Compute and store integral output from current integral accumulator.
     */
    void updateIntegralOutput();
    /**
     * Compute and store derivative output from current error delta.
     */
    void updateDerivativeOutput();
    /**
     * Compute and store the sum of P, I, and D outputs as the current output.
     */
    void updateOutputSum();
    /**
     * Compute and store proportional output using error delta.
     */
    void updateProportionalOutputFromErrorDelta();
    /**
     * Compute and store integral output using current error.
     */
    void updateIntegralOutputFromError();
    /**
     * Compute and store derivative output using the derivative buffer.
     */
    void updateDerivativeOutputFromDBuf();
    /**
     * Add the sum of output components to the current output accumulator.
     */
    void addOutputSum();
    /**
     * Update error from target and current measurement.
     */
    void updateErrorFromTargetAndMeasure();
    /**
     * Store current error as last error.
     */
    void updateLastError();
    /**
     * Run the direct PID update (P/I/D from current error).
     */
    void runDirectPidStep();
    /**
     * Run the incremental PID update (error buffer, d buffer, incremental sum).
     */
    void runIncrementalPidStep();
    /**
     * Run one PID update step and return the computed output.
     * @param measure Current measured value.
     * @return Computed PID output.
     */
    float pidCalculate(float measure);
};
