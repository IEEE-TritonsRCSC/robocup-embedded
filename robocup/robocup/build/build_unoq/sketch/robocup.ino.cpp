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

#include <Arduino_RouterBridge.h>

#define PORT 4210
#define BUFFER_SIZE 256
#define READABLE_BUFFER_SIZE BUFFER_SIZE - 1
#define NULL_TERMINATOR '\0'

BridgeUDP<> udp(Bridge);
char packetBuffer[BUFFER_SIZE];

#line 21 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 28 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 21 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
  Bridge.begin();
  Monitor.begin();
  udp.begin(PORT);
  Monitor.println("Ready");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, READABLE_BUFFER_SIZE);
    packetBuffer[len] = NULL_TERMINATOR;
    Monitor.print("Received: ");
    Monitor.println(packetBuffer);
  }
}
