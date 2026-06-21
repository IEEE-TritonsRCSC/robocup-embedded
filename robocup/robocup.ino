#pragma once

#include "RobotState.h"

static RobotState* state;

void setup() {
  pinMode(KICKER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("started"));

  SPI.begin();
   
  state = new RobotState();
}

void loop() {
   state->receiveCommand();
   state->executeState();
}
