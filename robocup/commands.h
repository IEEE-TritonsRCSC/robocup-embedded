#pragma once
#include <MoteusAcan2517fd.h>
#include "helpers.h"
#include "WiFi.h"
#include "WiFiUdp.h"

typedef Moteus::PositionMode::Command PositionCommand;

#define ROBOT_ID 1  // possible values are 1-6

#define WATCHDOG_TIMEOUT 4000 // in milliseconds

#define NUM_MOTORS 5
#define NUM_WHEELS 4

#define FL_WHEEL_INDEX 0
#define FR_WHEEL_INDEX 1
#define BR_WHEEL_INDEX 2
#define BL_WHEEL_INDEX 3
#define DRIBBLER_INDEX 4  // true dribbler index

#define FL_WHEEL_ANGLE 0.349066 // in radians
#define FR_WHEEL_ANGLE -0.349066 // in radians
#define BR_WHEEL_ANGLE 1.0472 // in radians
#define BL_WHEEL_ANGLE -1.0472 // in radians

// MOTOR CONFIG
#define STOP_MOTOR 0
#define START_VELOCITY STOP_MOTOR  // setup motors to start with 0 velocity
#define MAX_VELOCITY 6             // TODO: test the max velocity value on a robot
#define PURE_VELOCITY_MODE NaN     // set PositionCommand.position to this for pure velocity mode
#define MAX_TORQUE 0.29            // units: Nm // TODO: check this value later
#define IGNORE_POSITION_BOUNDS 1   // ignore position_max and position_min
#define DRIBBLER_SPEED 10          // TODO: test what speed is optimal
#define DRIBBLER_STOP STOP_MOTOR

#define MIN_DASH_POWER 0
#define MAX_DASH_POWER 100

#define DASH_CMD_CHAR 'd'
#define TURN_CMD_CHAR 't'
#define CATCH_CMD_CHAR 'c'
#define DROP_CMD_CHAR 'o'
#define KICK_CMD_CHAR 'k'
#define STOP_CMD_CHAR 'q'

#define DASH_NUM_ARGS 2
#define TURN_NUM_ARGS 1
#define CATCH_NUM_ARGS 0
#define DROP_NUM_ARGS 0
#define KICK_NUM_ARGS 0
#define STOP_NUM_ARGS 0

#define KICKER_PIN 13  // change to true kicker pin later

// MCP2517 pins for CAN FD Arduino Shield
#define MCP2517_SCK 13  // SCK
#define MCP2517_SDI 11  // SDI (MOSI)
#define MCP2517_SDO 12  // SDO (MISO)
#define MCP2517_CS 9    // CS or SS
#define MCP2517_INT 2   // INT (A)

// CANFD CONFIG
#define CANFD_BITRATE 1000ll * 1000ll  // 1 MBit bitrate for CANFD


/**
 * @brief initializes position commands for each motor in the setup
 * @param MotorCommands the CANFD Moteus Position Commands for each motor
 */
void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief set the driver buffer sizes for minimal memory usage
 */
void configCANFDSettings(ACAN2517FDSettings& settings);

/**
 * @brief invert the left wheels' motor rotation
 * @param WheelCommands position commmands for each wheel motor
 */
void invertLeftWheelsRotation(PositionCommand* WheelCommands[NUM_WHEELS]);

/** 
 * @brief Sends the MotorCommands to Motors
 * @param Motors The motor objects
 * @param MotorCommands position commands for each wheel motor
*/
void sendPositionCommands(Moteus* Motors[NUM_MOTORS], PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief print the motor velocities
 * @param MotorCommands position commands for each wheel motor
 */
void printMotorVelocities(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief print the motor velocities and overwrite the current line in the monitor
 * @param MotorCommands position commands for each wheel motor
 */
void printMotorVelocitiesInline(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief look at the parsed arguments from a UDP packet and execute the command
 */
bool executeUdpCommand(
   const int robotId,
   const char commandChar,
   const float arg1,
   const float arg2,
   PositionCommand* WheelCommands[NUM_WHEELS],
   PositionCommand* MotorCommands[NUM_MOTORS],
   unsigned long &lastUdpCommandMs,
   bool &watchdogStopped
);

/**
 * @brief parse an incoming UDP packet and execute the matching command
 */
void handleUdpPackets(
   WiFiUDP &udp,
   PositionCommand* WheelCommands[NUM_WHEELS],
   PositionCommand* MotorCommands[NUM_MOTORS],
   unsigned long &lastUdpCommandMs,
   bool &watchdogStopped
);

/**
 * @brief set the velocity for each wheel to dash in a specified direction with a specified power
 * @param power 0-100 value for the speed, where 100 is 1 m/s
 * @param direction some angle in degrees
 * @param WheelCommands position commmands for each wheel motor
 * @note direction can be a negative angle
 */
void dash(float power, float direction, PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief set the velocity for each wheel to turn with a specified angular velocity
 * @param turnSpeed rotational velocity in degrees/s
 * @param WheelCommands position commmands for each wheel motor
 * @warning the speed is not certain until tested on a robot 
 */
void turn(const float turnSpeed, PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief spin the dribbler at DRIBBLER_SPEED
 * @param MotorCommands position commands for each motor
 */
void dribblerCatch(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief stop the dribbler motor
 * @param MotorCommands position commands for each motor
 */
void dribblerDrop(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief stop the wheel motors
 * @param WheelCommands position commands for each wheel motor
 */
void stopLocomotion(PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief stop all motors
 * @param MotorCommands position commands for each motor
 */
void stop(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief turn kicker solenoid off
 * @param kickerPin GPIO pin for activating kicker solenoid
 */
void stopKicker(const byte kickerPin);

/**
 * @brief activate kicker solenoid for 100ms to kick ball
 * @param kickerPin GPIO pin for activating kicker solenoid
 */
void kick(const byte kickerPin);