#include <Arduino_RouterBridge.h>

const int UDP_PORT = 4210;
BridgeUDP<> udp(Bridge);
char packetBuffer[256];

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