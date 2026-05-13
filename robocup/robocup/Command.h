/**
 * Command.h
 * this is for functions for parsing commands from the packet buffer
 */

// BEGIN INCLUDES
#include "WiFiControl.h"
#include "Robot.h"
// END INCLUDES

// BEGIN DEFINES
// format specifiers for using sscanf
#define TWO_ARGS "%f %f"
#define ONE_ARG "%f"

#define CMD_PREFIX "%d %c"
#define TWO_ARGS_FORMAT CMD_PREFIX + TWO_ARGS
#define ONE_ARGS_FORMAT CMD_PREFIX + ONE_ARG
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

// END FUNCTION DECLARATION