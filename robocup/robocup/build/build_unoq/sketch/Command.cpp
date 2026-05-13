#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\Command.cpp"
/**
 * Command.cpp
 * Implements the command-parsing functions and helpers
 * 
 */

// BEGIN INCLUDES
#include "Command.h"
#include <string.h>
// END INCLUDES

// BEGIN FUNCTION DEFINITIONS

static float parseFloatToken(const char *token) {
   if (token == nullptr) {
      return 0.0f;
   }

   bool isNegative = false;
   if (*token == '-') {
      isNegative = true;
      token++;
   } else if (*token == '+') {
      token++;
   }

   float value = 0.0f;
   while (*token >= '0' && *token <= '9') {
      value = (value * 10.0f) + static_cast<float>(*token - '0');
      token++;
   }

   if (*token == '.') {
      token++;
      float placeValue = 0.1f;
      while (*token >= '0' && *token <= '9') {
         value += static_cast<float>(*token - '0') * placeValue;
         placeValue *= 0.1f;
         token++;
      }
   }

   if (isNegative) {
      value = -value;
   }

   return value;
}

// TODO: integrate this function into CommandData
void parsePacketIntoCommandData(char packetBuffer[BUFFER_SIZE], CommandData &commandData) {
   if (!isMatchingRobotID(packetBuffer) || !isValidCommand(packetBuffer)) {
      return;
   }

   char parseBuffer[BUFFER_SIZE];
   strncpy(parseBuffer, packetBuffer, BUFFER_SIZE - 1);
   parseBuffer[BUFFER_SIZE - 1] = '\0';

   char *robotIDToken = strtok(parseBuffer, " \t\r\n");
   char *commandToken = strtok(nullptr, " \t\r\n");
   char *arg1Token = strtok(nullptr, " \t\r\n");
   char *arg2Token = strtok(nullptr, " \t\r\n");

   if (robotIDToken == nullptr || commandToken == nullptr) {
      return;
   }

   char command = commandToken[0];
   float arg1 = 0;
   float arg2 = 0;

   // keep order of switch statement the same as `validCommands`
   switch (command) {
      case STOP_CMD:
         commandData.setStop(true); // TODO: replace with a const or arg
         return;
      case KICK_CMD:
         commandData.setKick(true); // TODO: replace with a const or arg
         return;
      case CATCH_CMD:
         commandData.setCatch(true);
         return;
      case SHORTKICK_CMD:
         if (arg1Token == nullptr) {
            return;
         }
         arg1 = parseFloatToken(arg1Token);
         commandData.setShortKick(arg1);
         return;
      case DASH_CMD:
         if (arg1Token == nullptr || arg2Token == nullptr) {
            return;
         }
         arg1 = parseFloatToken(arg1Token);
         arg2 = parseFloatToken(arg2Token);
         commandData.setDash(arg1,arg2);
         return;
      case TURN_CMD:
         if (arg1Token == nullptr) {
            return;
         }
         arg1 = parseFloatToken(arg1Token);
         commandData.setTurn(arg1);
         return;
      default:
         // unknown command
         return;
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
