#pragma once

#include "MotorControl.h"

void dash(Moteus* wheels[NUM_WHEELS], float wheelAngles[NUM_WHEELS], bool invertRotation[NUM_WHEELS], float dashPower, float direction) {
   direction *= PI / 180; // convert direction from degrees into radians
   Moteus::PositionMode::Command cmd;
   cmd.position = NaN;  // Pure velocity mode.
   for(int i=0;i<TEST_NUM_MOTORS;i++) {
      cmd.velocity = dashPower * cos(wheelAngles[i] - direction);
      if (invertRotation[i]) {
         cmd.velocity *= -1;
      }
      wheels[i]->BeginPosition(cmd);
   }
}

void turn(Moteus* wheels[NUM_WHEELS], float turnSpeed) {
   constexpr float arbitraryMultipler = 1; // this is for tuning during testing on a robot to match the specified rotational velocity
   Moteus::PositionMode::Command cmd;
   cmd.position = NaN;  // Pure velocity mode.
   for(int i=0;i<TEST_NUM_MOTORS;i++) {
      // convert from deg/s to wheel rev/s
      // deg/s / 360deg * robot circumference / robot circumference * 3 wheel revs per robot circumference = wheel rev/s
      cmd.velocity = -turnSpeed * arbitraryMultipler / 120; // convert rotational speed of robot to rotational speed of motors
      wheels[i]->BeginPosition(cmd);
   }
}


void dribblerCatch(Moteus* motors[NUM_MOTORS]) {
  constexpr float invertDribblerRotation = 1; // change to -1 if dribbler spins the wrong way
  constexpr float dribblerSpeed = 10; // this is for tuning the optimal dribbler speed during testing
  Moteus::PositionMode::Command cmd;
  cmd.position = NaN;  // Pure velocity mode.
  cmd.velocity = invertDribblerRotation * dribblerSpeed;
  motors[TEST_DRIBBLER_INDEX]->BeginPosition(cmd);
}

void stopDribbler(Moteus* motors[NUM_MOTORS]) {
  motors[DRIBBLER_INDEX]->BeginBrake();
}


void stopLocomotion(Moteus* motors[NUM_WHEELS]) {
  for(int i=0;i<NUM_WHEELS;i++) {
    motors[i]->BeginBrake();
  }
}


void shortKick(Moteus* motors[NUM_MOTORS], float shortKickPower) {
  constexpr float invertDribblerRotation = -1; // change to 1 if dribbler spins the wrong way
  Moteus::PositionMode::Command cmd;
  cmd.position = NaN;  // Pure velocity mode.
  cmd.velocity = invertDribblerRotation * shortKickPower;
  motors[TEST_DRIBBLER_INDEX]->BeginPosition(cmd);
}


void stopKicker(const byte kickerPin) {
   digitalWrite(kickerPin, LOW);
}


void kick(const byte kickerPin) {
   digitalWrite(kickerPin, HIGH);
   // delay 100ms without blocking
   digitalWrite(kickerPin, LOW);
}


void stop(Moteus* motors[NUM_MOTORS], const byte kickerPin) {
   stopLocomotion(motors);
   stopDribbler(motors); // commented out for testing because treating a wheel motor as a dribbler motor bugs it out
   stopKicker(kickerPin);
}