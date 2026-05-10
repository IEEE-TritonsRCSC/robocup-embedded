#include <Arduino.h>
#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
#include <Arduino_RouterBridge.h>

const int UDP_PORT = 4210;
BridgeUDP<> udp(Bridge);
char packetBuffer[256];

#line 7 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup();
#line 14 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void loop();
#line 7 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\robocup.ino"
void setup() {
  Bridge.begin();
  Monitor.begin();
  udp.begin(UDP_PORT);
  Monitor.println("Ready");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    packetBuffer[len] = '\0';
    Monitor.print("Received: ");
    Monitor.println(packetBuffer);
  }
}
