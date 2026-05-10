/**
 * This file defines the functions of LEDControl.h
 * The functions should make usage of the onboard LED easy and readable
 */

// BEGIN INCLUDES
#include "LEDControl.h"
// END INCLUDES

// BEGIN FUNCTION DEFINITION
void initLED() {
   pinMode(LED_BUILTIN, OUTPUT);
}

void initLEDoff() {
   initLED();
   LEDoff();
}

void initLEDon() {
   initLED();
   LEDon();
}

void LEDon() {
   digitalWrite(LED_BUILTIN,LED_ON);
}

void LEDoff() {
   digitalWrite(LED_BUILTIN,LED_OFF);
}

void blocking_blink(const unsigned short duration, const unsigned short numBlinks) {
   for (int i=0;i<numBlinks;i++) {
      LEDon();
      delay(duration);
      LEDoff();
      delay(duration);
   }
}
// END FUNCTION DEFINITION