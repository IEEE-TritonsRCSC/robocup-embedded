/**
 * @file TODO: write file header
 */

#include "oled.h"


void i2cScanner(TwoWire& Wire) {
  Serial.println("Scanning for I2C devices...");
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(i, HEX);
    }
  }
}