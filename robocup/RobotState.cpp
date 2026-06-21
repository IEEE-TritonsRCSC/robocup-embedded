#include "RobotState.h"

/**
   @brief angle of wheel relative to the front to back axis in radians
   @note order: FrontLeft, FrontRight, BackRight, BackLeft
   @note front angle is 20 degrees and back angle is 60 degrees
*/
constexpr float angles[NUM_WHEELS] = {
   0.349066,
   -0.349066,
   1.0472,
   -1.0472,
};

/**
   @brief should the velocity of the motor be inverted rotation
   @note order: FrontLeft, FrontRight, BackRight, BackLeft
   @note the left wheels must have inverted rotation
*/
constexpr bool invert[NUM_WHEELS] = {
   true,
   false,
   false,
   true
};

RobotState* RobotState::instance = nullptr;

void RobotState::handleCanInterrupt() {
  if (instance != nullptr) {
    instance->can.isr();
  }
}

RobotState::RobotState()
  : can(MCP2517_CS, SPI, MCP2517_INT),
    settings(
      ACAN2517FDSettings::OSC_20MHz,
      CANFD_BITRATE,
      DataBitRateFactor::x1
    ),
    wheelAngles(angles),
    wheelInvertedRotation(invert) {
  instance = this;

  // Keep the driver buffers small to fit on memory-constrained boards.
  settings.mArbitrationSJW = 2;
  settings.mDriverTransmitFIFOSize = 1;
  settings.mDriverReceiveFIFOSize = 2;

  const uint32_t errorCode = can.begin(settings, RobotState::handleCanInterrupt);
  while (errorCode != 0) {
    Serial.print(F("CAN error 0x"));
    Serial.println(errorCode, HEX);
    delay(1000);
  }

  for (int i = 0; i < NUM_MOTORS; i++) {
    motors[i] = new Moteus(can, [i]() {
        Moteus::Options options;
        options.id = i+1;
        return options;
    }());
    // Clear any faults
    motors[i]->BeginStop();
  }
}

void RobotState::dash(const float dashPower, const float dashDirection) {
  float direction = dashDirection * PI / 180.0f;
  MotorCommand cmd;
  cmd.position = NaN;
  for (int i = 0; i < NUM_MOTORS; ++i) {
    cmd.velocity = dashPower * cos(wheelAngles[i] - direction);
    if (wheelInvertedRotation[i]) {
      cmd.velocity *= -1;
    }
    motors[i]->BeginPosition(cmd);
  }
  isDashing = true;
  isTurning = false;
}

void RobotState::turn(const float turnSpeed) {
  constexpr float arbitraryMultipler = 1;
  MotorCommand cmd;
  cmd.position = NaN;
  for (int i = 0; i < NUM_MOTORS; ++i) {
    cmd.velocity = -turnSpeed * arbitraryMultipler / 120;
    motors[i]->BeginPosition(cmd);
  }
  isTurning = true;
  isDashing = false;
}

void RobotState::kick() {
  digitalWrite(KICKER_PIN, HIGH);
  isKicking = true;
}

void RobotState::stopKick() {
  digitalWrite(KICKER_PIN, LOW);
  isKicking = false;
}

void RobotState::dribblerCatch() {
  constexpr float invertDribblerRotation = 1;
  constexpr float dribblerSpeed = 10;
  MotorCommand cmd;
  cmd.position = NaN;
  cmd.velocity = invertDribblerRotation * dribblerSpeed;
  motors[DRIBBLER_INDEX]->BeginPosition(cmd);
  isCatching = true;
}

void RobotState::stopDribbler() {
  motors[DRIBBLER_INDEX]->BeginBrake();
  isCatching = false;
}

void RobotState::stopLocomotion() {
  for (int i = 0; i < NUM_WHEELS; ++i) {
    motors[i]->BeginBrake();
  }
  isDashing = false;
  isTurning = false;
}

void RobotState::stop() {
  stopLocomotion();
  stopDribbler();
  stopKick();
}
