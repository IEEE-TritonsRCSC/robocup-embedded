/**
 * @file commands.h
 * @brief Command parsing and motion-control helpers for the robot firmware.
 *
 * This header defines the command vocabulary, hardware layout constants, and
 * helper APIs used to translate UDP packets into wheel, dribbler, and kicker
 * actuator targets.
 */
#pragma once
#include <MoteusAcan2517fd.h>
#include "helpers.h"
#include <WiFiS3.h>
#include "WiFiUdp.h"

typedef Moteus::PositionMode::Command PositionCommand;

//////// BEGIN CONFIGURATION CONSTS
#ifndef ENABLE_MOTORS
#define ENABLE_MOTORS 1 // set to 1 to enable motor code
#endif

#ifndef ENABLE_TEST_MOTORS
#define ENABLE_TEST_MOTORS 1  // Enable the reduced test-motor layout by default.
#endif

#if ENABLE_TEST_MOTORS == 1 && !defined(NUM_TEST_MOTORS)
#define NUM_TEST_MOTORS 1
#endif

#define ROBOT_ID 1            // Valid values are 1-6.
#define WATCHDOG_TIMEOUT 4000 // Milliseconds before the robot auto-stops.

#define MAX_VELOCITY 6      // Maximum command velocity sent to each motor.
#define MAX_TORQUE 0.29     // Maximum torque limit in N·m.
#define DRIBBLER_SPEED 10   // Commanded dribbler velocity when catching.

/*
KICK-DIS D2
DONE D1
CHARGE D0
*/
#define KICKER_PIN 2

//////// END CONFIGURATION CONSTS

//////// BEGIN IMMUTABLE CONSTS
#if ENABLE_TEST_MOTORS == 1
   // In test mode, compile only the requested number of motor slots.
   #define NUM_MOTORS NUM_TEST_MOTORS
   #define NUM_WHEELS NUM_TEST_MOTORS
#else
   // Full robot layout: four wheels plus one dribbler.
   #define NUM_MOTORS 5
   #define NUM_WHEELS 4
#endif

#define FL_WHEEL_INDEX 0
#define FR_WHEEL_INDEX 1
#define BR_WHEEL_INDEX 2
#define BL_WHEEL_INDEX 3
#define DRIBBLER_INDEX 4  // true dribbler index
#define HAS_DRIBBLER (NUM_MOTORS > DRIBBLER_INDEX)

#define FL_WHEEL_ANGLE 0.349066 // in radians
#define FR_WHEEL_ANGLE -0.349066 // in radians
#define BR_WHEEL_ANGLE 1.0472 // in radians
#define BL_WHEEL_ANGLE -1.0472 // in radians

#define STOP_MOTOR 0
#define START_VELOCITY STOP_MOTOR
#define PURE_VELOCITY_MODE NaN
#define IGNORE_POSITION_BOUNDS 1
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

// MCP2517 pins for the CAN-FD Arduino shield.
#define MCP2517_SCK 13  // SCK
#define MCP2517_SDI 11  // SDI (MOSI)
#define MCP2517_SDO 12  // SDO (MISO)
#define MCP2517_CS 9    // CS or SS
#define MCP2517_INT 2   // INT (A)

// CAN-FD bitrate used by the drivetrain bus.
#define CANFD_BITRATE 1000ll * 1000ll
//////// END IMMUTABLE CONSTS

//////// BEGIN DEBUG CONSTS
#ifndef ENABLE_UDP_DEBUG
#define ENABLE_UDP_DEBUG 1
#endif
//////// END DEBUG CONSTS

/**
 * @brief Initialize each motor command with safe default values.
 *
 * Every slot is placed in velocity mode with conservative limits so the robot
 * starts from a known-safe state before any UDP command is processed.
 *
 * @param MotorCommands CAN-FD position commands for each motor.
 */
void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Tune CAN-FD driver buffers for a memory-constrained board.
 *
 * The default driver FIFO sizes are larger than we need for this sketch, so
 * the buffer sizes are reduced to save RAM.
 *
 * @param settings CAN-FD settings object to modify in place.
 */
void configCANFDSettings(ACAN2517FDSettings& settings);

/**
 * @brief Invert the left-side wheel velocities to match drivetrain layout.
 *
 * The wheel frame is mirrored on the left side, so those velocities need to
 * be flipped after holonomic math is applied.
 *
 * @param WheelCommands Position commands for each wheel motor.
 */
void invertLeftWheelsRotation(PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief Send the current motor commands to all initialized motors.
 *
 * If a motor object has not been created yet, the function logs a message and
 * returns early.
 *
 * @param Motors Motor objects corresponding to each motor index.
 * @param MotorCommands Position commands to send.
 */
void sendPositionCommands(Moteus* Motors[NUM_MOTORS], PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Print the current motor velocities as a multi-line Serial snapshot.
 *
 * @param MotorCommands Position commands for each motor.
 */
void printMotorVelocities(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Print wheel velocities as a debug snapshot.
 *
 * @param WheelCommands Position commands for each wheel motor.
 */
void printWheelVelocities(PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief Print motor velocities in place on a single Serial console line.
 *
 * This is useful for live debugging in terminal programs that support carriage
 * return updates.
 *
 * @param MotorCommands Position commands for each motor.
 */
void printMotorVelocitiesInline(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Execute a parsed UDP command for the target robot.
 *
 * Only packets whose robot ID matches `ROBOT_ID` are applied. A successful
 * command refreshes the watchdog timer and clears the watchdog stop state.
 *
 * @param robotId Robot identifier from the command packet.
 * @param commandChar Command selector character.
 * @param arg1 First parsed argument.
 * @param arg2 Second parsed argument.
 * @param WheelCommands Wheel position commands to modify.
 * @param MotorCommands Full motor command array to modify.
 * @param lastUdpCommandMs Timestamp of the last valid UDP command.
 * @param watchdogStopped Watchdog state flag, cleared when a valid command arrives.
 * @return True if the command was recognized and applied; otherwise false.
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
 * @brief Parse an incoming UDP packet and dispatch the matching command.
 *
 * Expected packets use the format: `<robotId> <command> [arg1] [arg2]`.
 * Commands with too few arguments are rejected before execution.
 *
 * @param udp UDP socket to read from.
 * @param WheelCommands Wheel position commands to modify.
 * @param MotorCommands Full motor command array to modify.
 * @param lastUdpCommandMs Timestamp of the last valid UDP command.
 * @param watchdogStopped Watchdog state flag, cleared when a valid command arrives.
 */
void handleUdpPackets(
   WiFiUDP &udp,
   PositionCommand* WheelCommands[NUM_WHEELS],
   PositionCommand* MotorCommands[NUM_MOTORS],
   unsigned long &lastUdpCommandMs,
   bool &watchdogStopped
);

/**
 * @brief Print a compact UDP debug line with packet metadata.
 *
 * @param tag Short label describing the debug stage.
 * @param packetSize Size reported by parsePacket().
 */
void printUdpDebugHeader(const char* tag, int packetSize);

/**
 * @brief Print a received UDP payload as text and hex bytes.
 *
 * @param payload Raw packet buffer.
 * @param length Number of valid bytes in the payload.
 */
void printUdpDebugPayload(const char* payload, int length);

/**
 * @brief Set wheel velocities for a translational dash command.
 *
 * @param power Dash power in the range 0-100.
 * @param direction Travel direction in degrees.
 * @param WheelCommands Wheel position commands to modify.
 * @note `direction` may be negative.
 */
void dash(float power, float direction, PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief Set wheel velocities for an in-place turn command.
 *
 * @param turnSpeed Angular velocity in degrees per second.
 * @param WheelCommands Wheel position commands to modify.
 * @warning The scaling factor is still experimental and should be verified on hardware.
 */
void turn(const float turnSpeed, PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief Spin the dribbler forward at the configured catch speed.
 *
 * @param MotorCommands Full motor command array to modify.
 */
void dribblerCatch(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Stop the dribbler motor.
 *
 * @param MotorCommands Full motor command array to modify.
 */
void dribblerDrop(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Stop the wheel motors.
 *
 * @param WheelCommands Wheel position commands to modify.
 */
void stopLocomotion(PositionCommand* WheelCommands[NUM_WHEELS]);

/**
 * @brief Stop all motors, including the dribbler.
 *
 * @param MotorCommands Full motor command array to modify.
 */
void stop(PositionCommand* MotorCommands[NUM_MOTORS]);

/**
 * @brief Deactivate the kicker solenoid output.
 *
 * @param kickerPin GPIO pin used for the kicker driver.
 */
void stopKicker(const byte kickerPin);

/**
 * @brief Pulse the kicker solenoid to perform a kick.
 *
 * @param kickerPin GPIO pin used for the kicker driver.
 */
void kick(const byte kickerPin);
