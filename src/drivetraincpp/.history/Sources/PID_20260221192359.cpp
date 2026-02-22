// #include "pid.h"
#include "stm32f4xx.h"
#include "PID.h"

#define ABS(x)	((x>0)? x: -x)

#ifdef __cplusplus
extern "C" {
#endif


float pid_calculate(PID_TypeDef* pid, float measure)
{
	pid->measure = measure;
	pid->last_error = pid->error;
	pid->error = pid->target - pid->measure;
	pid->pout = pid->kp * pid->error;

	// Prevent integral windup
	pid->integral += pid->error;
	if(pid->integral > pid->IntegralLimit)
	{
		pid->integral = pid->IntegralLimit;
	}
	if(pid->integral < -(pid->IntegralLimit))
	{
		pid->integral = -pid->IntegralLimit;
	}
	pid->iout = pid->ki * pid->integral;

	pid->dout =  pid->kd * (pid->error - pid->last_error);

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
	/}

	
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

#ifdef __cplusplus
}
#endif
