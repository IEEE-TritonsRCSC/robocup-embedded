/**
 * @file RobotState.h
 * @brief RobotState is a class to hold all information about the current state of the robot and the functions to change its state
 * 
 * The robot state is read in a loop, where it executes the current state of the robot.
 * The UDP commands update the states and never activate anything directly (except stop).
 * The robot state updates after each command.
 * Next, the robot executes the functions from its state.
 * This allows for sending CANFD frames continuously to keep the motors moving 
 * 
 */
#pragma once

#include <MoteusAcan2517fd.h>
#include <WiFiUdp.h>
#include <stdio.h>


// command format: "<robot-id> <command-char> [arg1 float] [arg2 float]"
#define ROBOT_ID 1
#define ROBOT_ID_INDEX 0 
#define CMD_CHAR_INDEX 2


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

#define EXCESSIVE_DRIBBLE_TIME 10000 // 10 sec TODO: what is the max dribble time
#define KICK_DELAY 100 // wait 100ms for kicker

// BEGIN PINS

// MCP2517 pins for CAN FD Arduino Shield
#define MCP2517_SCK 13  // SCK
#define MCP2517_SDI 11  // SDI (MOSI)
#define MCP2517_SDO 12  // SDO (MISO)
#define MCP2517_CS 9   // CS (SS)
#define MCP2517_INT 2   // INT (A)

#define KICKER_PIN 13 // TODO: change to true kicker pin later
#define BALL_DETECTOR_PIN 13 // TODO: change to true ball detector pin later

// END PINS

#define CANFD_BITRATE 1000ll * 1000ll  // 1 MBit bitrate for CANFD
#define PORT 10000
#define BUFFER_SIZE 256
#define READABLE_BUFFER_SIZE BUFFER_SIZE - 1
#define NULL_TERMINATOR '\0'

typedef Moteus::PositionMode::Command MotorCommand;

class RobotState {
   private:
   static RobotState* instance;
   static void handleCanInterrupt();

   void readPacketIntoBuffer();
   int hasPacket();
   int packetLength(const int readableBufferSize);
   void nullTerminatePacketBuffer(const int packetLength);
   void printPacket();
   bool doesPacketMatchRobot();
   bool isValidCommandChar();
   bool isValidCommand();

   /**
      @note motor indices 0-3 are wheel motors
      @note motor index 4 is the dribbler motor
   */
   Moteus* motors[NUM_MOTORS] = {nullptr};
   WiFiUDP udp;
   char packetBuffer[BUFFER_SIZE] = {0};
   ACAN2517FD can;
   ACAN2517FDSettings settings;
   const float* wheelAngles;
   const bool* wheelInvertedRotation;

   /*
   TODO: add this block to the execute state function or something
      if (isKicking && millis() - startKick > KICK_DELAY) { // KICK_DELAY is 100-150ms
         stopKick();
      }
   */
   unsigned long startKick;

   /*
   TODO: add this block to the execute state function or something
      if (isCatching && millis() - startDribble > EXCESSIVE_DRIBBLE_TIME) {
         stopDribbler();
      }
   */
   unsigned long startDribble;
   

   bool isDashing = false;
   bool isTurning = false;
   bool isCatching = false;
   bool isKicking = false;

   bool hasBall = false;

   // units: rev/s
   float dashPower;
   // units: degrees
   float dashDirection;
   // units: deg/s
   float turnSpeed;


   public:
   /**
    * @brief create a robot state wrapper around the configured motors
    */
   RobotState();

   /**
    * @brief executes the commands for the current state of the robot and updates state
    * // TODO: implement a watchdog timer to stop from continuously executing
    */
   void executeState();

   /**
    * @brief receive a UDP command and update the state
    */
   void receiveCommand();

   /**
    * @brief drive the robot in a chosen direction at the requested power
    * @note sets isTurning to false
    */
   void dash();

   /**
    * @brief rotate the robot in place at the requested speed
    */
   void turn();

   /**
    * @brief activate the kicker solenoid
    */
   void kick();

   /**
    * @brief stop the kicker output
    */
   void stopKick();

   /**
    * @brief spin the dribbler motor in the catch direction
    */
   void dribblerCatch();

   /**
    * @brief brake the dribbler motor
    */
   void stopDribbler();

   /**
    * @brief brake all locomotion motors
    */
   void stopLocomotion();

   /**
    * @brief stop locomotion, dribbler, and kicker output
    */
   void stop();

   void checkBallDetector();

   void setDashPower(const float power);

   void setDashDirection(const float direction);

   void setTurnSpeed(const float speed);

   /**
    * @brief check whether the robot is currently in dash mode
    * @return true if the robot is dashing, otherwise false
    */
   bool GetIsDashing() const;

   /**
    * @brief set whether the robot is currently in dash mode
    * @param dashing true to mark the robot as dashing, false otherwise
    */
   void SetIsDashing(const bool dashing);

   /**
    * @brief check whether the robot is currently in turn mode
    * @return true if the robot is turning, otherwise false
    */
   bool GetIsTurning() const;

   /**
    * @brief set whether the robot is currently in turn mode
    * @param turning true to mark the robot as turning, false otherwise
    */
   void SetIsTurning(const bool turning);

   /**
    * @brief check whether the robot is currently trying to catch the ball
    * @return true if the robot is catching, otherwise false
    */
   bool GetIsCatching() const;

   /**
    * @brief set whether the robot is currently trying to catch the ball
    * @param catching true to mark the robot as catching, false otherwise
    */
   void SetIsCatching(const bool catching);

   /**
    * @brief check whether the robot is currently kicking
    * @return true if the robot is kicking, otherwise false
    */
   bool GetIsKicking() const;

    /**
     * @brief set whether the robot is currently kicking
     * @param kicking true to mark the robot as kicking, false otherwise
     */
    void SetIsKicking(const bool kicking);

   /**
    * @brief check whether the robot currently has possession of the ball
    * @return true if the robot has the ball, otherwise false
    */
   bool GetHasBall() const;

   /**
    * @brief set whether the robot currently has possession of the ball
    * @param ball true to mark the robot as holding the ball, false otherwise
    */
   void SetHasBall(const bool ball);
};
