#include "PID.h"

  
  PID::PID(float Kp, float Kd, float Ki, float integral_limit, float target){
    this->Kp = Kp;
    this->Kd = Kd;
    this->Ki = Ki;
    this->integral_limit = integral_limit;
    this->target = target;
  }

  void PID::set_pid_constants(float Kp, float Kd, float Ki){
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
  }

  void PID::set_target(float new_target){
    this->target = new_target;
  }

  float PID::pid_calc(float measure){
    float error = this->target - measure;
    float pout = this->Kp * error;

    this->integral += error;
    if (this->integral > this->integral_limit){
      this->integral = this->integral_limit;

    } else if(this->integral < -(this->integral_limit)){
      this->integral = - (this->integral_limit);
    }
    float iout = this->Ki * this->integral;
    float dout = this->Kd * (error - this->last_error);
    float output = pout + iout + dout;
    this->last_error = error;
    return output;
  }

