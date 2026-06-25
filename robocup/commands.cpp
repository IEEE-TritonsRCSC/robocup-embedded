#include "commands.h"

void invertLeftWheelsRotation(PositionCommand* WheelCommands[NUM_WHEELS]) {
  WheelCommands[FL_WHEEL_INDEX]->velocity *= -1;
  WheelCommands[BL_WHEEL_INDEX]->velocity *= -1;
}

void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    MotorCommands[i]->position = PURE_VELOCITY_MODE;
    MotorCommands[i]->velocity = START_VELOCITY;
    MotorCommands[i]->maximum_torque = MAX_TORQUE;  // might be unnecessary
    MotorCommands[i]->ignore_position_bounds = IGNORE_POSITION_BOUNDS;
    MotorCommands[i]->velocity_limit = MAX_VELOCITY;
  }
}

void configCANFDSettings(ACAN2517FDSettings& settings) {
  // Keep the driver buffers small to fit on memory-constrained boards.
  settings.mArbitrationSJW = 2;
  settings.mDriverTransmitFIFOSize = 1;
  settings.mDriverReceiveFIFOSize = 2;
}

void dash(float power, float direction, PositionCommand* WheelCommands[NUM_WHEELS]) {
  direction = degToRad(direction);
  power = map(power,MIN_DASH_POWER,MAX_DASH_POWER,STOP_MOTOR,MAX_VELOCITY);
  for (int i = 0; i < NUM_WHEELS; i++) {
   switch (i) {
      case FL_WHEEL_INDEX:
         WheelCommands[i]->velocity = power * cos(FL_WHEEL_ANGLE - direction);
         break;
      case FR_WHEEL_INDEX:
         WheelCommands[i]->velocity = power * cos(FR_WHEEL_ANGLE - direction);
         break;
      case BR_WHEEL_INDEX:
         WheelCommands[i]->velocity = power * cos(BR_WHEEL_ANGLE - direction);
         break;
      case BL_WHEEL_INDEX:
         WheelCommands[i]->velocity = power * cos(BL_WHEEL_ANGLE - direction);
         break;
      default:
         Serial.println("Dash attempted to access an invalid motor index!");
         return;
   }
  }
  invertLeftWheelsRotation(WheelCommands);
}

void turn(const float turnSpeed, PositionCommand* WheelCommands[NUM_WHEELS]) {
  constexpr float arbitraryMultipler = 1; // TODO: test this on a robot
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = -degPerSecondToRPS(turnSpeed) * arbitraryMultipler;
  }
}

void dribblerCatch(PositionCommand* MotorCommands[NUM_MOTORS]) {
  MotorCommands[DRIBBLER_INDEX]->velocity = DRIBBLER_SPEED;
}

void dribblerDrop(PositionCommand* MotorCommands[NUM_MOTORS]) {
  MotorCommands[DRIBBLER_INDEX]->velocity = DRIBBLER_STOP;
}

void stopLocomotion(PositionCommand* WheelCommands[NUM_WHEELS]) {
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = STOP_MOTOR;
  }
}

void stop(PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    MotorCommands[i]->velocity = STOP_MOTOR;
  }
}

void stopKicker(const byte kickerPin) {
  digitalWrite(kickerPin, LOW);
}

void kick(const byte kickerPin) {
  digitalWrite(kickerPin, HIGH);
  // TODO: delay 100ms without blocking
  digitalWrite(kickerPin, LOW);
}