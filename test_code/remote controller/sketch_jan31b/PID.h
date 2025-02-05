#ifndef _PID_H
#define _PID_H

class PID {
  private:
    float Kp, Kd, Ki, integral_limit, target, integral, last_error;

  public: 
    PID(float Kp, float Kd, float Ki, float integral_limit, float target);
    void set_pid_constants(float Kp, float Ki, float Kd);
    void set_target(float new_target);
    float pid_calc(float measure);
};

#endif