/**
 * Command.cpp
 * Implements the command-parsing functions and helpers
 * 
 */

// BEGIN INCLUDES
#include "Command.h"
// END INCLUDES

// BEGIN FUNCTION DEFINITIONS
bool isMatchingRobotID(char packetBuffer[BUFFER_SIZE]) {
   int robotID = packetBuffer[ROBOT_ID_INDEX] - '0'; // - '0' turns the single digit char number into an integer value
   if (isValidRobotID(packetBuffer) && robotID == ROBOT_ID) {
      return true;
   } else {
      return false;
   }
}

bool isValidRobotID(char packetBuffer[BUFFER_SIZE]) {
   int robotID = packetBuffer[ROBOT_ID_INDEX] - '0'; // - '0' turns the single digit char number into an integer value
   if (robotID <= 0 || robotID > MAX_ROBOT_ID) {
      return false;
   } else {
      return true;
   }
}
// END FUNCTION DEFINITIONS