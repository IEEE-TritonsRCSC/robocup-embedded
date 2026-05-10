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



BridgeUDP<> udp(Bridge);
char packetBuffer[BUFFER_SIZE];

void setup() {
  Bridge.begin(); // start router bridge
  Monitor.begin(); // start serial monitor
  udp.begin(PORT); // start udp connection

  Monitor.println("Ready");
}

void loop() {
  parsePacket(udp, Monitor,packetBuffer); 
}