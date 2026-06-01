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

// it takes the linux on the UNO Q 30 seconds to boot up
constexpr unsigned int LINUX_BOOT_TIME = 30000;
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
  if (hasPacket(udp)) { // if a udp packet is available
    readPacketIntoBuffer(udp, Monitor,packetBuffer); // put the packet contents in the buffer
    if (isMatchingRobotID(packetBuffer)) { // if the packet matches this robot
      printPacket(Monitor,packetBuffer); // print packet contents
      // parse data from packet into command data
      parsePacketIntoCommandData(packetBuffer,commandData);
      commandData.printData(); // print the parsed command data values
    }
  }
}
