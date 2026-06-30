/**
 * @file commands.cpp
 * @brief Implementations for UDP command handling and actuator helpers.
 *
 * This file translates parsed command packets into wheel, dribbler, and kicker
 * outputs used by the main firmware loop.
 */
#include "commands.h"

void printUdpDebugHeader(const char* tag, int packetSize) {
#if ENABLE_UDP_DEBUG == 1
  Serial.print(F("[UDP] "));
  Serial.print(tag);
  Serial.print(F(" packetSize="));
  Serial.println(packetSize);
#else
  (void)tag;
  (void)packetSize;
#endif
}

void printUdpDebugPayload(const char* payload, int length) {
#if ENABLE_UDP_DEBUG == 1
  Serial.print(F("[UDP] raw=\""));
  for (int i = 0; i < length; i++) {
    const char c = payload[i];
    if (c == '\r') {
      Serial.print(F("\\r"));
    } else if (c == '\n') {
      Serial.print(F("\\n"));
    } else if (c == '\t') {
      Serial.print(F("\\t"));
    } else {
      Serial.print(c);
    }
  }
  Serial.println(F("\""));

  Serial.print(F("[UDP] hex="));
  for (int i = 0; i < length; i++) {
    if (i > 0) {
      Serial.print(' ');
    }
    const uint8_t b = static_cast<uint8_t>(payload[i]);
    if (b < 0x10) {
      Serial.print('0');
    }
    Serial.print(b, HEX);
  }
  Serial.println();
#else
  (void)payload;
  (void)length;
#endif
}

void invertLeftWheelsRotation(PositionCommand* WheelCommands[NUM_WHEELS]) {
  // The left-side motors are mirrored physically, so their velocity sign is flipped.
  // Only flip wheels that actually exist in the current build configuration.
  if (NUM_WHEELS > FL_WHEEL_INDEX && WheelCommands[FL_WHEEL_INDEX] != nullptr) {
    WheelCommands[FL_WHEEL_INDEX]->velocity *= -1;
  }
  if (NUM_WHEELS > BL_WHEEL_INDEX && WheelCommands[BL_WHEEL_INDEX] != nullptr) {
    WheelCommands[BL_WHEEL_INDEX]->velocity *= -1;
  }
}

void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    // Start each motor in velocity mode with conservative limits.
    MotorCommands[i]->position = PURE_VELOCITY_MODE;
    MotorCommands[i]->velocity = START_VELOCITY;
    MotorCommands[i]->maximum_torque = MAX_TORQUE;
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
  #if NUM_MOTORS == 1
    Serial.print(F("FL: "));
    Serial.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
  #elif NUM_MOTORS == 2
    Serial.print(F("FL: "));
    Serial.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
    Serial.print(F(" FR: "));
    Serial.println(MotorCommands[FR_WHEEL_INDEX]->velocity);
  #elif NUM_MOTORS == 3
    Serial.print(F("FL: "));
    Serial.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
    Serial.print(F(" FR: "));
    Serial.println(MotorCommands[FR_WHEEL_INDEX]->velocity);
    Serial.print(F(" BR: "));
    Serial.println(MotorCommands[BR_WHEEL_INDEX]->velocity);
  #elif NUM_MOTORS == 4
    Serial.print(F("FL: "));
    Serial.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
    Serial.print(F(" FR: "));
    Serial.println(MotorCommands[FR_WHEEL_INDEX]->velocity);
    Serial.print(F(" BR: "));
    Serial.println(MotorCommands[BR_WHEEL_INDEX]->velocity);
    Serial.print(F(" BL: "));
    Serial.println(MotorCommands[BL_WHEEL_INDEX]->velocity);
  #elif NUM_MOTORS == 5
    Serial.print(F("FL: "));
    Serial.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
    Serial.print(F(" FR: "));
    Serial.println(MotorCommands[FR_WHEEL_INDEX]->velocity);
    Serial.print(F(" BR: "));
    Serial.println(MotorCommands[BR_WHEEL_INDEX]->velocity);
    Serial.print(F(" BL: "));
    Serial.println(MotorCommands[BL_WHEEL_INDEX]->velocity);
    Serial.print(F(" Dribbler: "));
    Serial.println(MotorCommands[DRIBBLER_INDEX]->velocity);
  #endif
}

void printWheelVelocities(PositionCommand* WheelCommands[NUM_WHEELS]) {
  #if NUM_WHEELS >= 1 
  Serial.print(F("Wheel snapshot -> FL: "));
  Serial.print(WheelCommands[FL_WHEEL_INDEX]->velocity);
  #endif
  #if NUM_WHEELS >= 2
  Serial.print(F(" FR: "));
  Serial.print(WheelCommands[FR_WHEEL_INDEX]->velocity);
  #endif
  #if NUM_WHEELS >= 3
  Serial.print(F(" BR: "));
  Serial.print(WheelCommands[BR_WHEEL_INDEX]->velocity);
  #endif
  #if NUM_WHEELS >= 4
  Serial.print(F(" BL: "));
  Serial.println(WheelCommands[BL_WHEEL_INDEX]->velocity);
  #endif
}

void printMotorVelocitiesInline(PositionCommand* MotorCommands[NUM_MOTORS]) {
  static size_t previousLength = 0;

  #if NUM_MOTORS == 1
    const char* format = "FL: %.3f";
  #elif NUM_MOTORS == 2
    const char* format = "FL: %.3f FR: %.3f";
  #elif NUM_MOTORS == 3
    const char* format = "FL: %.3f FR: %.3f BR: %.3f";
  #elif NUM_MOTORS == 4
    const char* format = "FL: %.3f FR: %.3f BR: %.3f BL: %.3f";
  #elif NUM_MOTORS == 5
    const char* format = "FL: %.3f FR: %.3f BR: %.3f BL: %.3f Dribbler: %.3f";
  #endif

  char line[96];
  const int written = snprintf(
    line,
    sizeof(line),
    format,
    MotorCommands[FL_WHEEL_INDEX]->velocity
  #if NUM_MOTORS >= 2
    , MotorCommands[FR_WHEEL_INDEX]->velocity
  #endif
  #if NUM_MOTORS >= 3
    , MotorCommands[BR_WHEEL_INDEX]->velocity
  #endif
  #if NUM_MOTORS >= 4
    , MotorCommands[BL_WHEEL_INDEX]->velocity
  #endif
  #if NUM_MOTORS >= 5
    , MotorCommands[DRIBBLER_INDEX]->velocity
  #endif
  );

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
  // Remove leading whitespace in place.
  while (*text != '\0' && isspace(static_cast<unsigned char>(*text))) {
    text++;
  }

  if (*text == '\0') {
    return text;
  }

  // Remove trailing whitespace in place.
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
      #if ENABLE_MOTORS == 1
      #if HAS_DRIBBLER
      dribblerCatch(MotorCommands);
      #else
      Serial.println(F("Ignoring dribbler catch: dribbler not configured"));
      return false;
      #endif
      #else
      Serial.println(F("Ignoring dribbler catch: motors are disabled"));
      return false;
      #endif
      break;
    case DROP_CMD_CHAR:
      #if ENABLE_MOTORS == 1
      #if HAS_DRIBBLER
      dribblerDrop(MotorCommands);
      #else
      Serial.println(F("Ignoring dribbler drop: dribbler not configured"));
      return false;
      #endif
      #else
      Serial.println(F("Ignoring dribbler drop: motors are disabled"));
      return false;
      #endif
      break;
    case STOP_CMD_CHAR:
      #if ENABLE_MOTORS == 1
      stop(MotorCommands);
      #else
      Serial.println(F("Ignoring stop command: motors are disabled"));
      return false;
      #endif
      break;
    default:
      return false;
  }

  lastUdpCommandMs = millis();
  watchdogStopped = false;
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

  printUdpDebugHeader("parsePacket()", packetSize);

#if ENABLE_UDP_DEBUG == 1
  Serial.print(F("[UDP] from "));
  Serial.print(udp.remoteIP());
  Serial.print(F(":"));
  Serial.println(udp.remotePort());
#endif

  char packetBuffer[64];
  const int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
  printUdpDebugHeader("read()", len);
  if (len <= 0) {
    return;
  }
  packetBuffer[len] = '\0';
  printUdpDebugPayload(packetBuffer, len);
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

#if ENABLE_UDP_DEBUG == 1
  Serial.print(F("[UDP] tokens="));
  Serial.println(tokenCount);
  for (int i = 0; i < tokenCount; i++) {
    Serial.print(F("[UDP] token["));
    Serial.print(i);
    Serial.print(F("]=\""));
    Serial.print(tokens[i]);
    Serial.println(F("\""));
  }
#endif

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
  if (!executed) {
    Serial.print(F("[UDP] command rejected. targetROBOT_ID="));
    Serial.print(ROBOT_ID);
    Serial.print(F(" receivedRobotId="));
    Serial.print(robotId);
    Serial.print(F(" commandChar="));
    Serial.println(commandChar);
  }
  if (executed) {
    printWheelVelocities(WheelCommands);
    printMotorVelocities(MotorCommands);
  }
}

void dash(float power, float direction, PositionCommand* WheelCommands[NUM_WHEELS]) {
  // Convert the command into the robot's internal units.
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

  // Only apply left-wheel inversion when those wheels are compiled in.
  if (NUM_WHEELS >= 4) {
    invertLeftWheelsRotation(WheelCommands);
  }
}

void turn(const float turnSpeed, PositionCommand* WheelCommands[NUM_WHEELS]) {
  // The current scale factor is intentionally simple and still hardware-tuned.
  constexpr float arbitraryMultipler = 1;
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = -degPerSecondToRPS(turnSpeed) * arbitraryMultipler;
  }
}

void dribblerCatch(PositionCommand* MotorCommands[NUM_MOTORS]) {
  #if HAS_DRIBBLER
  MotorCommands[DRIBBLER_INDEX]->velocity = DRIBBLER_SPEED;
  #endif
}

void dribblerDrop(PositionCommand* MotorCommands[NUM_MOTORS]) {
  #if HAS_DRIBBLER
  MotorCommands[DRIBBLER_INDEX]->velocity = DRIBBLER_STOP;
  #endif
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
