#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\Robot.h"
/**
 * Robot.h
 * information and data structures for the robot data
 */

#pragma once

// BEGIN INCLUDES
#include <Arduino_RouterBridge.h>
// END INCLUDES

// BEGIN CONSTEXPR
constexpr unsigned int ROBOT_ID = 1;
constexpr unsigned int NUM_WHEELS = 4;
constexpr unsigned int NUM_MOTORS = 5;
// END CONSTEXPR

// BEGIN STRUCT DECLARATION
/**
 * @brief contains data of the state of the robot that updates from commands
 */
typedef struct {
   float dashPower;
   float dashDirection;
   float shortKickPower;
   float turnSpeed;
   bool dribble;
   bool kick;
   bool stop;
} CommandData_T;
// END STRUCT DECLARATION

// BEGIN STRUCT FUNCTION DECLARATION

/**
 * @brief Resets all command data fields to their default inactive values.
 * @param commandData The command data struct to clear.
 */
void clearCommandData(CommandData_T &commandData);

/**
 * @brief Clears existing command data and sets the stop command flag.
 * @param commandData The command data struct to update.
 */
void setStop(CommandData_T &commandData);

/**
 * @brief Sets the kick command flag.
 * @param commandData The command data struct to update.
 */
void setKick(CommandData_T &commandData);

/**
 * @brief Sets the dribbler state to catch and hold the ball.
 * @param commandData The command data struct to update.
 */
void setCatch(CommandData_T &commandData);

/**
 * @brief Sets the dash command power and movement direction.
 * @param commandData The command data struct to update.
 * @param power The dash power to apply.
 * @param direction The dash direction to move in.
 */
void setDash(CommandData_T &commandData, const float power, const float direction);

/**
 * @brief Sets the turning speed command.
 * @param commandData The command data struct to update.
 * @param speed The turn speed to apply.
 */
void setTurn(CommandData_T &commandData, const float speed);

/**
 * @brief Sets the short kick power command.
 * @param commandData The command data struct to update.
 * @param power The short kick power to apply.
 */
void setShortKick(CommandData_T &commandData, const float power);

void executeStop(const CommandData_T &commandData);
void executeKick(const CommandData_T &commandData);
void executeCatch(const CommandData_T &commandData);
void executeDash(const CommandData_T &commandData);
void executeTurn(const CommandData_T &commandData);
void executeShortKick(const CommandData_T &commandData);

/**
 * @brief print contents of a command data
 * @param commandData command data struct of robot
 */
void printData(const CommandData_T &commandData);
// END STRUCT FUNCTION DECLARATION
