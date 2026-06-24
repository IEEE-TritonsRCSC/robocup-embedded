#pragma once
#include <MoteusAcan2517fd.h>
#include "WiFi.h"
#include "WiFiUdp.h"

typedef Moteus::PositionMode::Command PositionCommand;

#define BAUD_RATE 115200

#define WIFI_SSID "wlan3"
#define WIFI_PASSWORD "a1b2c3d4"
#define LOCAL_IP_ADDRESS IPAddress(192, 168, 68, 50)
#define GATEWAY_IP_ADDRESS IPAddress(192, 168, 68, 1)
#define SUBNET_MASK IPAddress(255, 255, 255, 0)

#define UDP_PORT 10000

#define ROBOT_ID 1  // possible values are 1-6

#define WATCHDOG_TIMEOUT 4000 // in milliseconds

#define ROBOT_DIAMETER 0.2 // in meters
#define WHEEL_DIAMETER 0.06 // in meters
#define ROBOT_CIRCUMFERENCE ROBOT_DIAMETER * PI // in meters
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI // in meters
#define ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO ROBOT_CIRCUMFERENCE / WHEEL_CIRCUMFERENCE // in meters per meter

#define DASH_CMD_CHAR 'd'
#define TURN_CMD_CHAR 't'
#define CATCH_CMD_CHAR 'c'
#define DROP_CMD_CHAR 'o'
#define KICK_CMD_CHAR 'k'
#define STOP_CMD_CHAR 'q'

#define DASH_NUM_ARGS 2
#define TURN_NUM_ARGS 1
#define CATCH_NUM_ARGS 0
#define DROP_NUM_ARGS 0
#define KICK_NUM_ARGS 0
#define STOP_NUM_ARGS 0


// COMMAND TEST ARGS
#define TURN_SPEED -90  // in degrees
#define DASH_POWER 1
#define DASH_ANGLE 30  // in degrees

#define NUM_MOTORS 5
#define NUM_WHEELS 4
#define DRIBBLER_INDEX 4  // true dribbler index

#define MIN_DASH_POWER 0
#define MAX_DASH_POWER 100

// MOTOR CONFIG
#define STOP_MOTOR 0
#define START_VELOCITY STOP_MOTOR  // setup motors to start with 0 velocity
#define MAX_VELOCITY 6             // TODO: test the max velocity value on a robot
#define PURE_VELOCITY_MODE NaN     // set PositionCommand.position to this for pure velocity mode
#define MAX_TORQUE 0.29            // units: Nm // TODO: check this value later
#define IGNORE_POSITION_BOUNDS 1   // ignore position_max and position_min
#define DRIBBLER_SPEED 10          // TODO: test what speed is optimal
#define DRIBBLER_STOP STOP_MOTOR

// PINS
#define KICKER_PIN 13  // change to true kicker pin later
// MCP2517 pins for CAN FD Arduino Shield
#define MCP2517_SCK 13  // SCK
#define MCP2517_SDI 11  // SDI (MOSI)
#define MCP2517_SDO 12  // SDO (MISO)
#define MCP2517_CS 9    // CS or SS
#define MCP2517_INT 2   // INT (A)

// CANFD CONFIG
#define CANFD_BITRATE 1000ll * 1000ll  // 1 MBit bitrate for CANFD

// convert units from degrees to radians
constexpr float degToRad(const float angle) {
  return angle * PI / 180;
}

// convert units from deg/s to rev/s for turn function
constexpr float degPerSecondToRPS(const float degPerSecond) {
  return (degPerSecond * ROBOT_TO_WHEEL_CIRCUMFERENCE_RATIO) / 360;
}

/**
 * @brief turn kicker solenoid off
 * @param kickerPin GPIO pin for activating kicker solenoid
 */
void stopKicker(const byte kickerPin) {
  digitalWrite(kickerPin, LOW);
}

/**
 * @brief activate kicker solenoid for 100ms to kick ball
 * @param kickerPin GPIO pin for activating kicker solenoid
 */
void kick(const byte kickerPin) {
  digitalWrite(kickerPin, HIGH);
  // delay 100ms without blocking
  digitalWrite(kickerPin, LOW);
}

/**
 * @brief set the driver buffer sizes for minimal memory usage
 */
void configCANFDSettings(ACAN2517FDSettings& settings) {
  // Keep the driver buffers small to fit on memory-constrained boards.
  settings.mArbitrationSJW = 2;
  settings.mDriverTransmitFIFOSize = 1;
  settings.mDriverReceiveFIFOSize = 2;
}

static WiFiUDP udp;
static ACAN2517FD can(MCP2517_CS, SPI, MCP2517_INT);

/**
   @brief angle of wheel relative to the front to back axis in radians
   @note order: FrontLeft, FrontRight, BackRight, FrontLeft
   @note front angle is 20 degrees and back angle is 60 degrees
*/
float angles[NUM_WHEELS] = {
  0.349066,
  -0.349066,
  1.0472,
  -1.0472,
};


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

// invert the rotation of the left wheels
void invertLeftWheelsRotation() {
  FrontLeftWheelCmd.velocity *= -1;
  BackLeftWheelCmd.velocity *= -1;
}

/**
 * @brief initializes position commands for each motor in the setup
 * @param MotorCommands the CANFD Moteus Position Commands for each motor
 */
void initPositionCommands(PositionCommand* MotorCommands[NUM_MOTORS]) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    MotorCommands[i]->position = PURE_VELOCITY_MODE;
    MotorCommands[i]->velocity = START_VELOCITY;
    MotorCommands[i]->maximum_torque = MAX_TORQUE;  // might be unnecessary
    MotorCommands[i]->ignore_position_bounds = IGNORE_POSITION_BOUNDS;
    MotorCommands[i]->velocity_limit = MAX_VELOCITY;
  }
}

/**
 * @brief set the velocity for each wheel to dash in a specified direction with a specified power
 * @param power 0-100 value for the speed, where 100 is 1 m/s
 * @param direction some angle in degrees
 * @note direction can be a negative angle
 */
void dash(float power, float direction) {
  direction = degToRad(direction);
  power = map(power,MIN_DASH_POWER,MAX_DASH_POWER,STOP_MOTOR,MAX_VELOCITY);
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = power * cos(angles[i] - direction);
  }
  invertLeftWheelsRotation();
}

/**
 * @brief set the velocity for each wheel to turn with a specified angular velocity
 * @param turnSpeed rotational velocity in degrees/s
 * @warning the speed is not certain until tested on a robot 
 */
void turn(const float turnSpeed) {
  constexpr float arbitraryMultipler = 1; // TODO: test this on a robot
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = -degPerSecondToRPS(turnSpeed) * arbitraryMultipler;
  }
}


void dribblerCatch() {
  DribblerCmd.velocity = DRIBBLER_SPEED;
}

void dribblerDrop() {
  DribblerCmd.velocity = DRIBBLER_STOP;
}

void stopLocomotion() {
  for (int i = 0; i < NUM_WHEELS; i++) {
    WheelCommands[i]->velocity = STOP_MOTOR;
  }
}

void stop() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    MotorCommands[i]->velocity = STOP_MOTOR;
  }
}

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
      dash(arg1, arg2);
      break;
    case TURN_CMD_CHAR:
      turn(arg1);
      break;
    case KICK_CMD_CHAR:
      kick(KICKER_PIN);
      break;
    case CATCH_CMD_CHAR:
      dribblerCatch();
      break;
    case DROP_CMD_CHAR:
      dribblerDrop();
      break;
    case STOP_CMD_CHAR:
      stop();
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
    stop();
    watchdogStopped = true;
  }

  printMotorVelocitiesInline();  // use PuTTY. Arduino Cannot use Carriage return printing correctly
}
