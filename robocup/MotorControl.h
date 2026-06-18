#pragma once
#include <MoteusAcan2517fd.h>


// BEGIN COMMAND ARGS

#define TURN_SPEED -90 // in degrees
#define DASH_POWER 1
#define DASH_ANGLE 30 // in degrees

// END COMMAND ARGS

#define TEST_NUM_MOTORS 2
#define NUM_MOTORS 5
#define NUM_WHEELS 4
#define TEST_DRIBBLER_INDEX 0 // dribbler index for testing
#define DRIBBLER_INDEX 4 // true dribbler index

// BEGIN PINS

// MCP2517 pins for CAN FD Arduino Shield
#define MCP2517_SCK 13  // SCK
#define MCP2517_SDI 11  // SDI (MOSI)
#define MCP2517_SDO 12  // SDO (MISO)
#define MCP2517_CS 10    // CS or SS
#define MCP2517_INT 2   // INT (A)

#define KICKER_PIN 13 // change to true kicker pin later

// END PINS

#define CANFD_BITRATE 1000ll * 1000ll  // 1 MBit bitrate for CANFD

/**
 * @brief convert a angle's units from degrees to radians
 * @param angleInDegrees an angle in units of degrees
 * @return the original angle in units of radians
 */
constexpr float degToRad(const float angleInDegrees) {
   return angleInDegrees * PI / 180;
}



/**
 * @brief move the robot in a direction with a velocity power
 * @param wheels array wheel motors
 * @param wheelAngles angle of the wheels relative to front to back axis in radians
 * @param invertRotation which wheels have inverted rotation from orientation
 * @param power velocity of motor in rev/s
 * @param direction direction of motion in degrees
 */
void dash(Moteus* wheels[NUM_WHEELS], const float wheelAngles[NUM_WHEELS], const bool invertRotation[NUM_WHEELS], const float dashPower, float direction);

/**
 * @brief rotate the robot at some rotational velocity
 * @param wheels array wheel motors
 * @param turnSpeed rotational velocity in degrees/s
 * @warning the speed is not certain until tested on a robot 
 */
void turn(Moteus* wheels[NUM_WHEELS], const float turnSpeed);

/**
 * @brief turn the dribbler motor on
 * @param motors array of motors
 * @warning the optimal dribbler speed is unknown until robot testing
*/
void dribblerCatch(Moteus* motors[NUM_MOTORS]);

/**
 * @brief apply brake to the dribbler motor
 * @param motors array of motors
*/
void stopDribbler(Moteus* motors[NUM_MOTORS]);

/**
 * @brief apply brake to all wheel motors
 * @param motors array of wheel motors
*/
void stopLocomotion(Moteus* motors[NUM_WHEELS]);

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

/**
 * @brief stop all motors and kicker
 * @param motors array of motors
 * @param kickerPin GPIO pin for activating kicker solenoid
 */
void stop(Moteus* motors[NUM_MOTORS], const byte kickerPin);