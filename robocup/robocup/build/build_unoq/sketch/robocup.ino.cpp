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



BridgeUDP<> udp(Bridge);
char packetBuffer[BUFFER_SIZE];

#line 20 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 29 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 20 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
  initLEDoff();
  Bridge.begin(); // start router bridge
  Monitor.begin(); // start serial monitor
  udp.begin(PORT); // start udp connection

  Monitor.println("Ready");
}

void loop() {
  parsePacket(udp, Monitor,packetBuffer); 
}
