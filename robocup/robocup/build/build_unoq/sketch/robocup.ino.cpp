#include <Arduino.h>
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
/**
 * upload code
 * unplug board completely
 * wait like 10 seconds
 * plug board in
 * wait until serial port is available
 * run the send.py file
 * the serial monitor should print the message
 */


#include "WiFiControl.h"
#include "LEDControl.h"

// BEGIN CONSTEXPR
constexpr unsigned int LINUX_BOOT_TIME = 30000; // it takes the linux on the UNO Q 30 seconds to boot up
// END CONSTEXPR

BridgeUDP<> udp(Bridge);
char packetBuffer[BUFFER_SIZE];

#line 22 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 41 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 22 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
  delay(LINUX_BOOT_TIME);

  initLEDon();

  while (!Bridge.begin()) { // start router bridge
    delay(1000);
  }

  while (!Monitor.begin()) { // start serial monitor
    delay(1000);
  }

  udp.begin(PORT); // start udp connection

  Monitor.println("Ready");
  LEDoff();
}

void loop() {
  if (hasPacket(udp)) {
    parsePacket(udp, Monitor,packetBuffer); 
    printPacket(Monitor,packetBuffer);
  }
}
