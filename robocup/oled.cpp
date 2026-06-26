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

void robotInfoPage(Adafruit_SSD1306 &display, WiFiClass &WiFi) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Robot ID: ");
  display.println(ROBOT_ID);
  display.print("UDP Port: ");
  display.println(UDP_PORT);
  display.println("Local IP: ");
  display.println(WiFi.localIP());
  display.println("Gateway IP: ");
  display.println(WiFi.gatewayIP());
  display.display();
}