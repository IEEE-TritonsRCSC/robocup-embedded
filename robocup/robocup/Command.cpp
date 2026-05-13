/**
 * Command.cpp
 * Implements the command-parsing functions and helpers
 * 
 */

// BEGIN INCLUDES
#include "Command.h"
// END INCLUDES

// BEGIN FUNCTION DEFINITIONS


void parsePacket(char packetBuffer[BUFFER_SIZE], CommandData_T &commandData) {
   int robotID;
   char command;
   float arg1, arg2;

   if (!isMatchingRobotID(packetBuffer) || !isValidCommand(packetBuffer)) {
      return;
   }

   sscanf(packetBuffer,TWO_ARGS_FORMAT,&robotID, &command, &arg1, &arg2);

   switch (command) {
      STOP_CMD:
         
         break;
      KICK_CMD:
         // TODO implement function for kick command
         break;
      CATCH_CMD:
         // TODO implement function for catch command
         break;
      SHORTKICK_CMD:
         // TODO implement function for short kick command
         break;
      DASH_CMD:
         // TODO implement function for dash command
         break;
      TURN_CMD:
         // TODO implement function for turn command
         break;
      default:
         // unknown command
         break;
   }
}


bool isValidCommand(const char packetBuffer[BUFFER_SIZE]) {
   char command = packetBuffer[CMD_INDEX];
   for (int i=0;i<NUM_CMDS;i++) {
      if (command == validCommands[i]) {
         return true;
      }
   }
   return false;
}

bool isMatchingRobotID(const char packetBuffer[BUFFER_SIZE]) {
   int robotID = packetBuffer[ROBOT_ID_INDEX] - '0'; // - '0' turns the single digit char number into an integer value
   if (isValidRobotID(packetBuffer) && robotID == ROBOT_ID) {
      return true;
   } else {
      return false;
   }
}

bool isValidRobotID(const char packetBuffer[BUFFER_SIZE]) {
   int robotID = packetBuffer[ROBOT_ID_INDEX] - '0'; // - '0' turns the single digit char number into an integer value
   if (robotID <= 0 || robotID > MAX_ROBOT_ID) {
      return false;
   } else {
      return true;
   }
}
// END FUNCTION DEFINITIONS