#include <Arduino.h>
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
/**
 * This file contains the main code for the Arduino Uno Q
 * The BEGIN and END comments are meant to organize the code
 * Please organize your code and write in the correct sections
 */
// BEGIN INCLUDES
#include "credentials.h"
#include "LEDControl.h"
// END INCLUDES

// BEGIN DEFINE

// END DEFINE

#line 15 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 22 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 15 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
   Serial.begin();

   initLED();

   Serial.println("Setup Done!");
}
void loop() {
   blocking_blink(250);
   Serial.println("BLINK");
}
