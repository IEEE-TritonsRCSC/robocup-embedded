#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\Command.h"
/**
 * Command.h
 * this is for functions for parsing commands from the packet buffer
 */

#pragma once

// BEGIN INCLUDES
#include "WiFiControl.h"
#include "Robot.h"
// END INCLUDES

// BEGIN DEFINES
// format specifiers for using sscanf
#define TWO_ARGS "%f %f"
#define ONE_ARG "%f"

#define CMD_PREFIX "%d %c"
#define TWO_ARGS_FORMAT "%d %c %f %f" 
#define ONE_ARGS_FORMAT "%d %c %f"
#define NO_ARG_FORMAT CMD_PREFIX
// END DEFINES

// BEGIN CONSTEXPR
constexpr unsigned int ROBOT_ID_INDEX = 0;
constexpr unsigned int CMD_INDEX = 2;

/**
 * @brief maximum possible Robot ID
 */
constexpr unsigned int MAX_ROBOT_ID = 6;

constexpr char DASH_CMD = 'd';
constexpr char TURN_CMD = 't';
constexpr char SHORTKICK_CMD = 's';
constexpr char KICK_CMD = 'k';
constexpr char CATCH_CMD = 'c';
constexpr char STOP_CMD = 'q';
constexpr int NUM_CMDS = 6;

/**
 * @brief list of possible command characters
 * @warning order the commands from most time-sensitive to least time-sensitive. 
 * The stop command needs to be checked extremely fast, so it's first. 
 * Kick must be executed quickly, so it's second. 
 * Turn is not as important, so it's last.
 */
constexpr const char validCommands[] = {
   STOP_CMD,
   KICK_CMD,
   CATCH_CMD,
   SHORTKICK_CMD,
   DASH_CMD,
   TURN_CMD
};

constexpr unsigned int DASH_NUM_ARGS = 2;
constexpr unsigned int TURN_NUM_ARGS = 1;
constexpr unsigned int SHORTKICK_NUM_ARGS = 1;
constexpr unsigned int KICK_NUM_ARGS = 0;
constexpr unsigned int CATCH_NUM_ARGS = 0;
constexpr unsigned int STOP_NUM_ARGS = 0;
// END CONSTEXPR


// BEGIN FUNCTION DECLARATION

/**
 * @brief check if the command in the packet buffer is one of the possible commands
 * @param packetBuffer holds the UDP command string
 * @return is the command one of the commands in `validCommands`?
 */
bool isValidCommand(const char packetBuffer[BUFFER_SIZE]);

/**
 * @brief parse command string in packet buffer and set command data accordingly
 * @param packetBuffer holds UDP command string
 * @param commandData holds state of robot and updates from commands
 */
void parsePacketIntoCommandData(char packetBuffer[BUFFER_SIZE], CommandData &commandData);

/**
 * @brief check if robot ID in packet buffer is valid and matches this robot's ID
 * @param packetBuffer holds a UDP packet with a command string
 */
bool isMatchingRobotID(const char packetBuffer[BUFFER_SIZE]);

/**
 * @brief check if robot ID in packet buffer is between 1 and maximum robot ID
 * @param packetBuffer holds a UDP packet with a command string
 */
bool isValidRobotID(const char packetBuffer[BUFFER_SIZE]);

// END FUNCTION DECLARATION