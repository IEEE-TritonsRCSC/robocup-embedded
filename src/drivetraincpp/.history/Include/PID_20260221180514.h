/**
 ******************************************************************************
 * @file		 pid.h
 * @author  Ginger
 * @version V1.0.0
 * @date    2015/11/14
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#ifndef _PID_H
#define _PID_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

class PID_Gains {
	private:

	float kp, ki, kd;

	public:

	PID_Gains(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd){}
	
	void setP(float p);
	void setI(float i);
	void setD(float d);

	float getP();
	float getI();
	float getD();
};

class PID_Outputs {
	private:

	float pout, iout, dout;

	public:

	PID_Outputs(float pout, float iout, float dout) : pout(pout), iout(iout), dout(dout){}

	void setPout(float p);
	void setIout(float i);
	void setDout(float d);

	float getPout();
	float getIout();
	float getDout();
};

/* typedef struct _PID_TypeDef {
} PID_TypeDef; */

class PID_Object {
	float target;
	float lastNoneZeroTarget;

	float measure;
	float error;
	float last_error;
	float integral;

	float d_buf[3];
	float error_buf[3];

	float output;

	float MaxOutput;
	float IntegralLimit;
	float DeadBand;

	uint32_t thistime;
	uint32_t lasttime;
	uint8_t dtime;

	PID_Object(float max)
};

void pid_init(PID_TypeDef *pid, float maxout, float integral_limit,
		float deadband, float target, float kp, float ki, float kd);
void pid_set_constants(PID_TypeDef *pid, float kp, float ki, float kd);
float pid_calculate(PID_TypeDef *pid, float measure);

extern PID_TypeDef motor_pid[4];

#ifdef __cplusplus
}
#endif

#endif

