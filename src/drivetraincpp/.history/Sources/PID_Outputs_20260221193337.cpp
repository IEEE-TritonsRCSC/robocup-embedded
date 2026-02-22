#include "PID_Gains.h"
#include "PID_Outputs.h"

PID_Outputs::PID_Outputs(float pout, float iout, float dout) : pout(pout), iout(iout), dout(dout) {}

PID_Outputs::PID_Outputs()
{
    this->pout = 0;
    this->iout = 0;
    this->dout = 0;
}
PID_Outputs::PID_Outputs() = default;

void PID_Outputs::setPout(float pout) { this->pout = pout; }
void PID_Outputs::setIout(float iout) { this->iout = iout; }
void PID_Outputs::setDout(float dout) { this->dout = dout; }

float PID_Outputs::getPout() { return pout; }
float PID_Outputs::getIout() { return iout; }
float PID_Outputs::getDout() { return dout; }

float PID_Outputs::getOutputSum() {
    return this->pout + this->iout + this->dout;
}