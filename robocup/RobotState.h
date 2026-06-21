#pragma once

#include "MotorControl.h"

class RobotState {
   private:
   Moteus* motors[NUM_MOTORS];
   const float* wheelAngles;
   const bool* wheelInvertedRotation;

   bool isDashing = false;
   bool isTurning = false;
   bool isCatching = false;
   bool isKicking = false;

   bool hasBall = false;


   public:
   /**
    * @brief create a robot state wrapper around the configured motors
    * @param motors array of motor pointers used by the robot
    * @param wheelAngles wheel heading angles in radians
    * @param wheelInvertedRotation wheel rotation inversion flags
    */
   RobotState(Moteus* motors[NUM_MOTORS], const float wheelAngles[NUM_WHEELS], const bool wheelInvertedRotation[NUM_WHEELS]);

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
