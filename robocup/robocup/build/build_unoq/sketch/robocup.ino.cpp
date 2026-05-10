#include <Arduino.h>
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
/**
 * This file contains the main code for the Arduino Uno Q
 * The BEGIN and END comments are meant to organize the code
 * Please organize your code and write in the correct sections
 */
// BEGIN INCLUDES
#include "LEDControl.h"
#include "WiFiControl.h"
// END INCLUDES

// BEGIN DEFINE
#define BLINK_DURATION 250 // duration of onboard LED blink in milliseconds
// END DEFINE

void setup() {
   Serial.begin();

   initLED();

   Serial.println("Setup Done!");
}
void loop() {
   blocking_blink(BLINK_DURATION);
   Serial.println("BLINK");
}
