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
typedef struct _PID_TypeDef
{
	float target;
	float lastNoneZeroTarget;
	float kp;
	float ki;
	float kd;

	float measure;
	float error;
	float last_error;
	float integral;

	float pout;
	float iout;
	float dout;

	float d_buf[3];
	float error_buf[3];

	float output;
	float last_output;

	float MaxOutput;
	float IntegralLimit;
	float DeadBand;

	uint32_t thistime;
	uint32_t lasttime;
	uint8_t dtime;
}PID_TypeDef;

	void pid_init(PID_TypeDef* pid, uint16_t maxout, uint16_t integral_limit, float deadband, int16_t  target, float kp, float ki, float kd);
	void pid_set_constants(PID_TypeDef *pid, float kp,float ki, float kd);
	float pid_calculate (PID_TypeDef *pid, float measure);
#endif

extern PID_TypeDef motor_pid[4];

