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

void robotInfoPage(Adafruit_SSD1306 &display, CWifi &WiFi) {
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

void positionCommandsPage(Adafruit_SSD1306 &display, PositionCommand* MotorCommands[NUM_MOTORS]) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("FL: ");
  display.println(MotorCommands[FL_WHEEL_INDEX]->velocity);
  display.print("FR: ");
  display.println(MotorCommands[FR_WHEEL_INDEX]->velocity);
  display.print("BR: ");
  display.println(MotorCommands[BR_WHEEL_INDEX]->velocity);
  display.print("BL: ");
  display.println(MotorCommands[BL_WHEEL_INDEX]->velocity);
  display.print("D: ");
  display.println(MotorCommands[DRIBBLER_INDEX]->velocity);
  display.display();
}
