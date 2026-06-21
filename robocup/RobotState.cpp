#include "RobotState.h"

RobotState::RobotState(
    Moteus* motors[NUM_MOTORS],
    const float wheelAngles[NUM_WHEELS],
    const bool wheelInvertedRotation[NUM_WHEELS])
    : wheelAngles(wheelAngles),
      wheelInvertedRotation(wheelInvertedRotation) {
  for (int i = 0; i < NUM_MOTORS; ++i) {
    this->motors[i] = motors[i];
  }
}

void RobotState::dash(const float dashPower, const float dashDirection) {
  float direction = dashDirection * PI / 180.0f;
  Moteus::PositionMode::Command cmd;
  cmd.position = NaN;
  for (int i = 0; i < TEST_NUM_MOTORS; ++i) {
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
  Moteus::PositionMode::Command cmd;
  cmd.position = NaN;
  for (int i = 0; i < TEST_NUM_MOTORS; ++i) {
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
  Moteus::PositionMode::Command cmd;
  cmd.position = NaN;
  cmd.velocity = invertDribblerRotation * dribblerSpeed;
  motors[TEST_DRIBBLER_INDEX]->BeginPosition(cmd);
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
