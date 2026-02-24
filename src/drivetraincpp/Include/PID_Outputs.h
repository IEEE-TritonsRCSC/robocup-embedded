#pragma once


class PID_Outputs
{
private:
    float pout, iout, dout;

public:
    PID_Outputs(float pout, float iout, float dout) : pout(pout), iout(iout), dout(dout) {}
    PID_Outputs();

    void setPout(float p);
    void setIout(float i);
    void setDout(float d);

    float getPout();
    float getIout();
    float getDout();

    float getOutputSum();
};
