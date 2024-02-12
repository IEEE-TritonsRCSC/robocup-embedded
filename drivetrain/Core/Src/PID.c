#include "pid.h"
#include "stm32f4xx.h"

#define ABS(x)	((x>0)? x: -x)

void pid_init(
	PID_TypeDef * pid,
	uint16_t maxout,
	uint16_t integral_limit,
	float deadband,
	int16_t  target,

	float 	kp,
	float 	ki,
	float 	kd)
{

	pid->MaxOutput = maxout;
	pid->IntegralLimit = integral_limit;
	pid->DeadBand = deadband;
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

float pid_calculate(PID_TypeDef* pid, float measure)
{
	pid->lasttime = pid->thistime;
	pid->thistime = HAL_GetTick();
	pid->dtime = pid->thistime-pid->lasttime;
	pid->measure = measure;
	pid->last_output = pid->output;

	pid->last_error  = pid->error;
	pid->error = pid->target - pid->measure;
	if((ABS(pid->error) > pid->DeadBand))
	{
		pid->pout = pid->kp * pid->error;

		//Integral with windup
		pid->iout += (pid->ki * pid->error * pid->dtime);
		if(pid->iout > pid->IntegralLimit)
			pid->iout = pid->IntegralLimit;
		if(pid->iout < - pid->IntegralLimit)
			pid->iout = - pid->IntegralLimit;

		pid->dout =  pid->kd * (pid->error - pid->last_error)/pid->dtime;

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
	}

	/*
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
	*/


	return pid->output;
}
