#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\Robot.cpp"
/**
 * Robot.cpp
 * implements the methods of the CommandData_T typedef struct
 */

// BEGIN INCLUDES
#include "Robot.h"
// END INCLUDES

// BEGIN STRUCT FUNCTION DEFINITIONS
void clearCommandData(CommandData_T &commandData) {
   commandData.dashPower = 0;
   commandData.dashDirection = 0;
   commandData.shortKickPower = 0;
   commandData.turnSpeed = 0;
   commandData.dribble = false;
   commandData.kick = false;
   commandData.stop = false;
}

void setStop(CommandData_T &commandData) {
   clearCommandData(commandData);
   commandData.stop = true;
}

void setKick(CommandData_T &commandData) {
   // TODO: check with mechanical whether kick is possible while dribbling
   commandData.kick = true;
}

void setCatch(CommandData_T &commandData) {
   commandData.dribble = true;
}

void setDash(CommandData_T &commandData, const float power, const float direction) {
   // TODO: make sure to check how to implement dashing while turning
   commandData.dashPower = power;
   commandData.dashDirection = direction;
}

void setTurn(CommandData_T &commandData, const float speed) {
   commandData.turnSpeed = speed;
}

void setShortKick(CommandData_T &commandData, const float power) {
   commandData.shortKickPower = power;
}

void executeStop(const CommandData_T &commandData);
void executeKick(const CommandData_T &commandData);
void executeCatch(const CommandData_T &commandData);
void executeDash(const CommandData_T &commandData);
void executeTurn(const CommandData_T &commandData);
void executeShortKick(const CommandData_T &commandData);

void printData(const CommandData_T &commandData) {
   Monitor.println("Command Data:");
   Monitor.print("Dash Power: ");
   Monitor.println(commandData.dashPower);
   Monitor.print("Dash Direction: ");
   Monitor.println(commandData.dashDirection);
   Monitor.print("Turn Speed: ");
   Monitor.println(commandData.turnSpeed);
   Monitor.print("Short Kick Power: ");
   Monitor.println(commandData.shortKickPower);
   Monitor.print("Catch: ");
   Monitor.println(commandData.dribble);
   Monitor.print("Kick: ");
   Monitor.println(commandData.kick);
   Monitor.print("Stop: ");
   Monitor.println(commandData.stop);
}
// END STRUCT FUNCTION DEFINITIONS
