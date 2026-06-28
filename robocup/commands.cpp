/**
 * @file commands.cpp
 * @brief Implementations for UDP command handling and actuator helpers.
 *
 * This file translates parsed command packets into wheel, dribbler, and kicker
 * outputs used by the main firmware loop.
 */
#include "commands.h"

void invertLeftWheelsRotation(PositionCommand* WheelCommands[NUM_WHEELS]) {
  WheelCommands[FL_WHEEL_INDEX]->velocity *= -1;
  WheelCommands[BL_WHEEL_INDEX]->velocity *= -1;
}

void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    // Start each motor in velocity mode with conservative limits.
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

void sendPositionCommands(Moteus* Motors[NUM_MOTORS], PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (Motors[i] == nullptr) {
      Serial.println(F("Skipping sendPositionCommands(): Motors not initialized"));
      return;
    }
    // Send the latest command to each motor one by one.
    Motors[i]->BeginPosition(*MotorCommands[i]);
  }
}

void printMotorVelocities(PositionCommand* MotorCommands[NUM_MOTORS]) {
  Serial.print(F("FL: "));
  Serial.print(MotorCommands[FL_WHEEL_INDEX]->velocity);
  Serial.print(F(" FR: "));
  Serial.print(MotorCommands[FR_WHEEL_INDEX]->velocity);
  Serial.print(F(" BR: "));
  Serial.print(MotorCommands[BR_WHEEL_INDEX]->velocity);
  Serial.print(F(" BL: "));
  Serial.print(MotorCommands[BL_WHEEL_INDEX]->velocity);
  Serial.print(F(" Dribbler: "));
  Serial.println(MotorCommands[DRIBBLER_INDEX]->velocity);
}

void printWheelVelocities(PositionCommand* WheelCommands[NUM_WHEELS]) {
  Serial.print(F("Wheel snapshot -> FL: "));
  Serial.print(WheelCommands[FL_WHEEL_INDEX]->velocity);
  Serial.print(F(" FR: "));
  Serial.print(WheelCommands[FR_WHEEL_INDEX]->velocity);
  Serial.print(F(" BR: "));
  Serial.print(WheelCommands[BR_WHEEL_INDEX]->velocity);
  Serial.print(F(" BL: "));
  Serial.println(WheelCommands[BL_WHEEL_INDEX]->velocity);
}

void printMotorVelocitiesInline(PositionCommand* MotorCommands[NUM_MOTORS]) {
  static size_t previousLength = 0;

  char line[96];
  const int written = snprintf(
    line,
    sizeof(line),
    "FL: %.3f FR: %.3f BR: %.3f BL: %.3f Dribbler: %.3f",
    MotorCommands[FL_WHEEL_INDEX]->velocity);
    // MotorCommands[FR_WHEEL_INDEX]->velocity,
    // MotorCommands[BR_WHEEL_INDEX]->velocity,
    // MotorCommands[BL_WHEEL_INDEX]->velocity,
    // MotorCommands[DRIBBLER_INDEX]->velocity);

  if (written < 0) {
    return;
  }

  const size_t currentLength = static_cast<size_t>(written);
  Serial.print('\r');
  Serial.print(line);

  if (previousLength > currentLength) {
    for (size_t i = currentLength; i < previousLength; i++) {
      Serial.print(' ');
    }
  }

  previousLength = currentLength;
}

static char* trimWhitespace(char* text) {
  while (*text != '\0' && isspace(static_cast<unsigned char>(*text))) {
    text++;
  }

  if (*text == '\0') {
    return text;
  }

  char* end = text + strlen(text) - 1;
  while (end > text && isspace(static_cast<unsigned char>(*end))) {
    *end = '\0';
    end--;
  }

  return text;
}

bool executeUdpCommand(
  const int robotId,
  const char commandChar,
  const float arg1,
  const float arg2,
  PositionCommand* WheelCommands[NUM_WHEELS],
  PositionCommand* MotorCommands[NUM_MOTORS],
  unsigned long &lastUdpCommandMs,
  bool &watchdogStopped
) {
  if (robotId != ROBOT_ID) {
    return false;
  }

  switch (commandChar) {
    case DASH_CMD_CHAR:
      dash(arg1, arg2, WheelCommands);
      break;
    case TURN_CMD_CHAR:
      turn(arg1, WheelCommands);
      break;
    case KICK_CMD_CHAR:
      kick(KICKER_PIN);
      break;
    case CATCH_CMD_CHAR:
      dribblerCatch(MotorCommands);
      break;
    case DROP_CMD_CHAR:
      dribblerDrop(MotorCommands);
      break;
    case STOP_CMD_CHAR:
      stop(MotorCommands);
      break;
    default:
      return false;
  }

  lastUdpCommandMs = millis();
  watchdogStopped = false;
  // printMotorVelocities();
  return true;
}

void handleUdpPackets(
  WiFiUDP &udp,
  PositionCommand* WheelCommands[NUM_WHEELS],
  PositionCommand* MotorCommands[NUM_MOTORS],
  unsigned long &lastUdpCommandMs,
  bool &watchdogStopped
) {
  // Ignore idle loops when no UDP packet is waiting.
  const int packetSize = udp.parsePacket();
  if (packetSize <= 0) {
    return;
  }

  char packetBuffer[64];
  const int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
  if (len <= 0) {
    return;
  }
  packetBuffer[len] = '\0';
  char* trimmedPacket = trimWhitespace(packetBuffer);
  char originalPacket[64];
  strncpy(originalPacket, trimmedPacket, sizeof(originalPacket) - 1);
  originalPacket[sizeof(originalPacket) - 1] = '\0';

  char* tokens[4] = { nullptr, nullptr, nullptr, nullptr };
  int tokenCount = 0;
  char* savePtr = nullptr;
  for (char* token = strtok_r(trimmedPacket, " \t", &savePtr);
       token != nullptr && tokenCount < 4;
       token = strtok_r(nullptr, " \t", &savePtr)) {
    tokens[tokenCount++] = token;
  }

  if (tokenCount < 2) {
    Serial.print(F("Bad UDP command: "));
    Serial.println(originalPacket);
    return;
  }

  const int robotId = atoi(tokens[0]);
  const char commandChar = tokens[1][0];
  const float arg1 = (tokenCount >= 3) ? atof(tokens[2]) : 0.0f;
  const float arg2 = (tokenCount >= 4) ? atof(tokens[3]) : 0.0f;

  Serial.print(F("UDP packet: '"));
  Serial.print(originalPacket);
  Serial.print(F("' parsed="));
  Serial.print(tokenCount);
  Serial.print(F(" robotId="));
  Serial.print(robotId);
  Serial.print(F(" command="));
  Serial.print(commandChar);
  Serial.print(F(" arg1="));
  Serial.print(arg1);
  Serial.print(F(" arg2="));
  Serial.println(arg2);

  if ((commandChar == DASH_CMD_CHAR && tokenCount < 4) || (commandChar == TURN_CMD_CHAR && tokenCount < 3) || (commandChar == KICK_CMD_CHAR && tokenCount < 2) || (commandChar == CATCH_CMD_CHAR && tokenCount < 2) || (commandChar == DROP_CMD_CHAR && tokenCount < 2) || (commandChar == STOP_CMD_CHAR && tokenCount < 2)) {
    Serial.print(F("Incomplete UDP command: "));
    Serial.println(originalPacket);
    return;
  }

  const bool executed = executeUdpCommand(robotId, 
    commandChar, 
    arg1, 
    arg2, 
    WheelCommands, 
    MotorCommands, 
    lastUdpCommandMs, 
    watchdogStopped
  );

  Serial.print(F("UDP command executed: "));
  Serial.println(executed ? F("yes") : F("no"));
  if (executed) {
    printWheelVelocities(WheelCommands);
    printMotorVelocities(MotorCommands);
  }
}

void dash(float power, float direction, PositionCommand* WheelCommands[NUM_WHEELS]) {
  direction = degToRad(direction);
  power = ((power - MIN_DASH_POWER) * (MAX_VELOCITY - STOP_MOTOR)) /
          (MAX_DASH_POWER - MIN_DASH_POWER) + STOP_MOTOR;
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
