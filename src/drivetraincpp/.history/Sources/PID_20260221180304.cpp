// #include "pid.h"
#include "stm32f4xx.h"
#include "PID.h"

#define ABS(x)	((x>0)? x: -x)

#ifdef __cplusplus
extern "C" {
#endif

PID_Gains::PID_Gains(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd){}

void PID_Gains::setP(float p) {
	this->kp = p;
}
void PID_Gains::setI(float i) {
	this->ki = i;
}
void PID_Gains::setD(float d) {
	this->kd = d;
}

float PID_Gains::getP() {return kp;}
float PID_Gains::getI() {return ki;}
float PID_Gains::getD() {return kd;}

PID_Outputs::PID_Outputs(float pout, float iout, float dout) : pout(pout), iout(iout), dout(dout) {}

void PID_Outputs::setPout(float pout) {this->pout = pout;}
void PID_Outputs::setIout(float iout) {this->iout = iout;}
void PID_Outputs::setDout(float dout) {this->dout = dout;}

float PID_Outputs::getPout() {return pout;}
float PID_Outputs::getIout() {return iout;}
float PID_Outputs::getDout() {return dout;}



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
	//}

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

#ifdef __cplusplus
}
#endif
