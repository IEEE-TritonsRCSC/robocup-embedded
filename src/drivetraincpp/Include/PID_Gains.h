#pragma once


class PID_Gains
{
private:
    float kp, ki, kd;

public:
    PID_Gains(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd) {}
    PID_Gains(float *gains_array);
    PID_Gains();

    void setP(float p);
    void setI(float i);
    void setD(float d);

    float getP();
    float getI();
    float getD();
};
