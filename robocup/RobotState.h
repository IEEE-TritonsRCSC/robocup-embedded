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
#define MCP2517_CS 9   // CS or SS
#define MCP2517_INT 2   // INT (A)

#define KICKER_PIN 13 // change to true kicker pin later

// END PINS

#define CANFD_BITRATE 1000ll * 1000ll  // 1 MBit bitrate for CANFD

typedef Moteus::PositionMode::Command MotorCommand;

class RobotState {
   private:
   static RobotState* instance;
   static void handleCanInterrupt();

   /**
      @note motor indices 0-3 are wheel motors
      @note motor index 4 is the dribbler motor
   */
   Moteus* motors[NUM_MOTORS] = {nullptr};
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


   public:
   /**
    * @brief create a robot state wrapper around the configured motors
    */
   RobotState();

   /**
    * @brief drive the robot in a chosen direction at the requested power
    * @param dashPower target drive power in revolutions per second
    * @param dashDirection direction of travel in degrees
    */
   void dash(const float dashPower, const float dashDirection);

   /**
    * @brief rotate the robot in place at the requested speed
    * @param turnSpeed rotational speed in degrees per second
    */
   void turn(const float turnSpeed);

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

   /**
    * @brief check whether the robot is currently in dash mode
    * @return true if the robot is dashing, otherwise false
    */
   bool GetIsDashing() const { return isDashing; }

   /**
    * @brief set whether the robot is currently in dash mode
    * @param dashing true to mark the robot as dashing, false otherwise
    */
   void SetIsDashing(const bool dashing) { isDashing = dashing; }

   /**
    * @brief check whether the robot is currently in turn mode
    * @return true if the robot is turning, otherwise false
    */
   bool GetIsTurning() const { return isTurning; }

   /**
    * @brief set whether the robot is currently in turn mode
    * @param turning true to mark the robot as turning, false otherwise
    */
   void SetIsTurning(const bool turning) { isTurning = turning; }

   /**
    * @brief check whether the robot is currently trying to catch the ball
    * @return true if the robot is catching, otherwise false
    */
   bool GetIsCatching() const { return isCatching; }

   /**
    * @brief set whether the robot is currently trying to catch the ball
    * @param catching true to mark the robot as catching, false otherwise
    */
   void SetIsCatching(const bool catching) { isCatching = catching; }

   /**
    * @brief check whether the robot is currently kicking
    * @return true if the robot is kicking, otherwise false
    */
   bool GetIsKicking() const { return isKicking; }

    /**
     * @brief set whether the robot is currently kicking
     * @param kicking true to mark the robot as kicking, false otherwise
     */
    void SetIsKicking(const bool kicking) { isKicking = kicking; }

   /**
    * @brief check whether the robot currently has possession of the ball
    * @return true if the robot has the ball, otherwise false
    */
   bool GetHasBall() const { return hasBall; }

   /**
    * @brief set whether the robot currently has possession of the ball
    * @param ball true to mark the robot as holding the ball, false otherwise
    */
   void SetHasBall(const bool ball) { hasBall = ball; }
};
