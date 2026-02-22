#include "PID_Data.h"

void setTarget(float target);
void setLastNoneZeroTarget(float lastNoneZeroTarget);
void setMeasure(float measure);
void setError(float error);
void setLastError(float last_error);
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