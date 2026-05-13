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

// BEGIN CLASS DECLARATION
/**
 * @brief contains data of the state of the robot that updates from commands
 */
class CommandData {
   private:
      float dashPower;
      float dashDirection;
      float shortKickPower;
      float turnSpeed;
      bool dribble;
      bool kick;
      bool stop;
   public:

      /**
       * @brief initialize all members to 0 and false
       */
      CommandData();

      /**
       * @brief set all members to 0 and false
       */
      void clearCommandData();

      /**
       * @brief Clears existing command data and sets the stop command flag.
       * @param isStopped flag to stop or let the robot continue
       */
      void setStop(const bool isStopped);

      /**
       * @brief Sets the kick command flag.
       * @param doKick flag to kick or disengage kicker
       */
      void setKick(const bool doKick);

      /**
       * @brief Sets the dribbler state to catch and hold the ball.
       * @param doCatch flag to catch or disengage dribbler
       */
      void setCatch(const bool doCatch);

      /**
       * @brief Sets the dash command power and movement direction.
       * @param power The dash power to apply.
       * @param direction The dash direction to move in.
       */
      void setDash(const float power, const float direction);

      /**
       * @brief Sets the turning speed command.
       * @param speed The turn speed to apply.
       */
      void setTurn(const float speed);

      /**
       * @brief Sets the short kick power command.
       * @param power The short kick power to apply.
       */
      void setShortKick(const float power);

      void executeStop();
      void executeKick();
      void executeCatch();
      void executeDash();
      void executeTurn();
      void executeShortKick();

      /**
       * @brief print contents of a command data
       */
      void printData();
};

// END CLASS DECLARATION
