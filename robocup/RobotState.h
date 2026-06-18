#pragma once

#include "MotorControl.h"

class RobotState {
   
   Moteus* motors[NUM_MOTORS];
   bool isDashing = false;
   bool isTurning = false;
   bool isCatching = false;
   bool isKicking = false;

   public:
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
};
