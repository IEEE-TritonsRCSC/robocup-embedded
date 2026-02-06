// #include "pid.h"
#include "stm32f4xx.h"
#include "PID.h"
#include <math.h>

#define ABS(x)	((x>0)? x: -x)

void pid_init(
	PID_TypeDef * pid,
	float maxout,
	float integral_limit,
	float deadband,
	float  target,

	float 	kp,
	float 	ki,
	float 	kd)
{

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

float pid_calculate(PID_TypeDef *pid, float measure)
{
	uint32_t current_time = HAL_GetTick();
	float dt = (current_time - pid->lasttime) / 1000.0f;
	pid->lasttime = current_time;

	pid->measure = measure;
	pid->last_error = pid->error;
	pid->error = pid->target - pid->measure;

	// Calculate proportional term
	pid->pout = pid->kp * pid->error;

	// Calculate integral term
	pid->integral += pid->error * dt;

	if (pid->integral > pid->IntegralLimit)
	{
		pid->integral = pid->IntegralLimit;
	}
	else if (pid->integral < -pid->IntegralLimit)
	{
		pid->integral = -pid->IntegralLimit;
	}
	pid->iout = pid->ki * pid->integral;

	// Calculate derivative term
	pid->dout = pid->kd * (pid->error - pid->last_error) / dt;

	// Friction compensation
	float friction_comp = 0.0f;
	float abs_measure = fabsf(pid->measure);

	// Only apply friction compensation when:
	// - We actually want to move (non-zero target)
	// - We're in the low-speed zone where static friction matters
	if (fabsf(pid->target) >= 1 && abs_measure < 1)
	{
		float smooth_factor = 1.0f - abs_measure;

		// Use reduced friction for crossing to prevent overshoot
		char is_crossing = (pid->target * pid->measure) < 0.0f;
		float max_friction = is_crossing ? 100.0f : 400.0f;

		friction_comp = smooth_factor * max_friction;
		if (pid->target < 0.0f)
		{
			friction_comp = -friction_comp;
		}
	}
	// pid->output = pid->pout + pid->iout + pid->dout + friction_comp;
	pid->output = pid->pout + friction_comp;

	if (pid->output > pid->MaxOutput)
	{
		pid->output = pid->MaxOutput;
	}
	else if (pid->output < -pid->MaxOutput)
	{
		pid->output = -pid->MaxOutput;
	}

	if (pid->target == 0.0f)
	{
		pid->output = 0.0f;
		pid->integral = 0.0f;
	}
	return pid->output;