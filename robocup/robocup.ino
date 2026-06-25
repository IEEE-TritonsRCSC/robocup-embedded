#pragma once

#include "commands.h"
#include "credentials.h"

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
  handleUdpPackets(udp, WheelCommands, MotorCommands, lastUdpCommandMs, watchdogStopped);

  const unsigned long now = millis();
  if (!watchdogStopped && lastUdpCommandMs != 0 && (now - lastUdpCommandMs >= WATCHDOG_TIMEOUT)) {
    Serial.println(F("WATCHDOG timeout: stopping robot"));
    stop(MotorCommands);
    watchdogStopped = true;
  }

  printMotorVelocitiesInline(MotorCommands);  // use PuTTY. Arduino Cannot use Carriage return printing correctly
}
