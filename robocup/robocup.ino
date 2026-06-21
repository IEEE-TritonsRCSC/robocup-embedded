#pragma once

#include "RobotState.h"

static RobotState* state;

void setup() {
  pinMode(KICKER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BALL_DETECTOR_PIN, INPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("started"));

  SPI.begin();
   
  state = new RobotState();
}

void loop() {
   state->checkBallDetector();
   state->receiveCommand();
   state->executeState();
   state->sendBallPossessionStatus();
}
