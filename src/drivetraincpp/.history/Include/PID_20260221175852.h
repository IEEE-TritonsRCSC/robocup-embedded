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
}



class PID

void pid_init(PID_TypeDef *pid, float maxout, float integral_limit,
		float deadband, float target, float kp, float ki, float kd);
void pid_set_constants(PID_TypeDef *pid, float kp, float ki, float kd);
float pid_calculate(PID_TypeDef *pid, float measure);

extern PID_TypeDef motor_pid[4];

#ifdef __cplusplus
}
#endif

#endif

