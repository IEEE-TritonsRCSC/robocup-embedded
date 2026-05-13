/**
 * upload code
 * unplug board completely
 * wait like 10 seconds
 * plug board in
 * wait until serial port is available
 * run the send.py file
 * the serial monitor should print the message
 */

#include "Robot.h"
#include "LEDControl.h"
#include "Command.h"

// BEGIN CONSTEXPR
constexpr unsigned int LINUX_BOOT_TIME = 30000; // it takes the linux on the UNO Q 30 seconds to boot up
// END CONSTEXPR

BridgeUDP<> udp(Bridge);
char packetBuffer[BUFFER_SIZE];
CommandData commandData;

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

  commandData.clearCommandData();

  Monitor.println("Ready");
  LEDoff();
}

void loop() {
  if (hasPacket(udp)) {
    readPacketIntoBuffer(udp, Monitor,packetBuffer);
    if (isMatchingRobotID(packetBuffer)) {
      printPacket(Monitor,packetBuffer);
      parsePacketIntoCommandData(packetBuffer,commandData);
      commandData.printData();
    }
  }
}
