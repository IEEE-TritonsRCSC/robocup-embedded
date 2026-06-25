#pragma once
#include "WiFi.h"
#include "WiFiUdp.h"
#include "commands.h"
#include "credentials.h"

#define ROBOT_ID 1  // possible values are 1-6

#define BAUD_RATE 115200

static WiFiUDP udp;
static ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

static Moteus* Motors[NUM_MOTORS]{ nullptr };
static Moteus* Wheels[NUM_WHEELS]{ nullptr };
static unsigned long lastUdpCommandMs = 0;
static bool watchdogStopped = false;

// Moteus CANFD Position Commands for each motor
static PositionCommand FrontLeftWheelCmd;
static PositionCommand FrontRightWheelCmd;
static PositionCommand BackRightWheelCmd;
static PositionCommand BackLeftWheelCmd;
static PositionCommand DribblerCmd;

static PositionCommand* MotorCommands[NUM_MOTORS] = {
  &FrontLeftWheelCmd,
  &FrontRightWheelCmd,
  &BackRightWheelCmd,
  &BackLeftWheelCmd,
  &DribblerCmd
};

static PositionCommand* WheelCommands[NUM_WHEELS] = {
  &FrontLeftWheelCmd,
  &FrontRightWheelCmd,
  &BackRightWheelCmd,
  &BackLeftWheelCmd,
};

// Sends the MotorCommands to Motors
void sendPositionCommands() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (Motors[i] == nullptr) {
      Serial.println(F("Skipping sendPositionCommands(): Motors not initialized"));
      return;
    }
    Motors[i]->BeginPosition(*MotorCommands[i]);
  }
}

void printMotorVelocities() {
  Serial.print(F("FL: "));
  Serial.print(FrontLeftWheelCmd.velocity);
  Serial.print(F(" FR: "));
  Serial.print(FrontRightWheelCmd.velocity);
  Serial.print(F(" BR: "));
  Serial.print(BackRightWheelCmd.velocity);
  Serial.print(F(" BL: "));
  Serial.print(BackLeftWheelCmd.velocity);
  Serial.print(F(" Dribbler: "));
  Serial.println(DribblerCmd.velocity);
}

void printMotorVelocitiesInline() {
  static size_t previousLength = 0;

  char line[96];
  const int written = snprintf(
    line,
    sizeof(line),
    "FL: %.3f FR: %.3f BR: %.3f BL: %.3f Dribbler: %.3f",
    FrontLeftWheelCmd.velocity,
    FrontRightWheelCmd.velocity,
    BackRightWheelCmd.velocity,
    BackLeftWheelCmd.velocity,
    DribblerCmd.velocity);

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

bool executeUdpCommand(
  const int robotId,
  const char commandChar,
  const float arg1,
  const float arg2) {
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

void handleUdpPackets() {
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

  int robotId = -1;
  char commandChar = '\0';
  float arg1 = 0.0f;
  float arg2 = 0.0f;

  const int parsed = sscanf(packetBuffer, "%d %c %f %f", &robotId, &commandChar, &arg1, &arg2);
  if (parsed < 2) {
    Serial.print(F("Bad UDP command: "));
    Serial.println(packetBuffer);
    return;
  }

  if ((commandChar == DASH_CMD_CHAR && parsed < 4) || (commandChar == TURN_CMD_CHAR && parsed < 3) || (commandChar == KICK_CMD_CHAR && parsed < 2) || (commandChar == CATCH_CMD_CHAR && parsed < 2) || (commandChar == DROP_CMD_CHAR && parsed < 2) || (commandChar == STOP_CMD_CHAR && parsed < 2)) {
    Serial.print(F("Incomplete UDP command: "));
    Serial.println(packetBuffer);
    return;
  }

  executeUdpCommand(robotId, commandChar, arg1, arg2);
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Serial Started!");

  Serial.println("Start Pins Init!");
  pinMode(KICKER_PIN, OUTPUT);
  //   pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Pins Initialized!");


  Serial.println("Start SPI!");
  SPI.begin();
  Serial.println("SPI Started!");

  Serial.println("Start Settings!");
  // Run CAN-FD at 1 Mbit/s for both arbitration and data.
  ACAN2517FDSettings settings(
    ACAN2517FDSettings::OSC_20MHz, CANFD_BITRATE, DataBitRateFactor::x1);
  Serial.println("Settings Started!");

  Serial.println("Config Settings!");
  configCANFDSettings(settings);
  Serial.println("Settings Configured!");

  Serial.println("Start Init Positon Commands!");
  initPositionCommands(MotorCommands);
  Serial.println("Position Commands Initialized!");

  WiFi.config(LOCAL_IP_ADDRESS, GATEWAY_IP_ADDRESS, SUBNET_MASK);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print(F("WiFi connected, IP: "));
  Serial.println(WiFi.localIP());

  udp.begin(UDP_PORT);
  Serial.print(F("UDP listening on port "));
  Serial.println(UDP_PORT);

  // start CAN communication and print error while disconnected
  /* const uint32_t errorCode = can.begin(settings, [] {
    can.isr();
  });
  while (errorCode != 0) {
    Serial.print(F("CAN error 0x"));
    Serial.println(errorCode, HEX);
    delay(1000);
  } */

  // create motor objects
  /*
   for (int i=0;i<NUM_MOTORS;i++) {
      Motors[i] = new Moteus(can, [i]() {
         Moteus::Options options;
         options.id = i+1;
         return options;
      }());
      // Clear any faults
      Motors[i]->BeginStop();
   } */

  Serial.println("Setup Done!");
  Serial.println("Current Velocities: ");
}

void loop() {
  // sendPositionCommands();
  handleUdpPackets();

  const unsigned long now = millis();
  if (!watchdogStopped && lastUdpCommandMs != 0 && (now - lastUdpCommandMs >= WATCHDOG_TIMEOUT)) {
    Serial.println(F("WATCHDOG timeout: stopping robot"));
    stop(MotorCommands);
    watchdogStopped = true;
  }

  printMotorVelocitiesInline();  // use PuTTY. Arduino Cannot use Carriage return printing correctly
}
