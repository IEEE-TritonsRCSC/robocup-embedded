/**
 * Robot.cpp
 * implements the methods of the CommandData class
 */

// BEGIN INCLUDES
#include "Robot.h"
// END INCLUDES

CommandData::CommandData() {
   this->dashPower = 0;
   this->dashDirection = 0;
   this->shortKickPower = 0;
   this->turnSpeed = 0;
   this->dribble = false;
   this->kick = false;
   this->stop = false;
}

// BEGIN CLASS FUNCTION DEFINITIONS
void CommandData::clearCommandData() {
   this->dashPower = 0;
   this->dashDirection = 0;
   this->shortKickPower = 0;
   this->turnSpeed = 0;
   this->dribble = false;
   this->kick = false;
   this->stop = false;
}

void CommandData::setStop(const bool isStopped) {
   clearCommandData();
   this->stop = isStopped;
}

void CommandData::setKick(const bool doKick) {
   // TODO: check with mechanical whether kick is possible while dribbling
   this->kick = doKick;
}

void CommandData::setCatch(const bool doCatch) {
   this->dribble = doCatch;
}

void CommandData::setDash(const float power, const float direction) {
   // TODO: make sure to check how to implement dashing while turning
   this->dashPower = power;
   this->dashDirection = direction;
}

void CommandData::setTurn(const float speed) {
   this->turnSpeed = speed;
}

void CommandData::setShortKick(const float power) {
   this->shortKickPower = power;
}

// void executeStop(); TODO: implement executors
// void executeKick();
// void executeCatch();
// void executeDash();
// void executeTurn();
// void executeShortKick();

void CommandData::printData() {
   Monitor.println("Command Data:");
   Monitor.print("Dash Power: ");
   Monitor.println(this->dashPower);
   Monitor.print("Dash Direction: ");
   Monitor.println(this->dashDirection);
   Monitor.print("Turn Speed: ");
   Monitor.println(this->turnSpeed);
   Monitor.print("Short Kick Power: ");
   Monitor.println(this->shortKickPower);
   Monitor.print("Catch: ");
   Monitor.println(this->dribble);
   Monitor.print("Kick: ");
   Monitor.println(this->kick);
   Monitor.print("Stop: ");
   Monitor.println(this->stop);
}
// END STRUCT FUNCTION DEFINITIONS
