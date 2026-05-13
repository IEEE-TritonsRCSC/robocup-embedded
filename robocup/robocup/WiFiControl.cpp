/**
 * This file defines the functions of WiFiControls.h
 * The functions makes WiFi features easier to use and read
 */

// BEGIN INCLUDES
#include "WiFiControl.h"
// END INCLUDES

// BEGIN FUNCTION DEFINITIONS
void parsePacket(
   BridgeUDP<> &udp, 
   BridgeMonitor<> &Monitor, 
   char packetBuffer[BUFFER_SIZE]) 
{
   int len = packetLength(udp,packetBuffer,READABLE_BUFFER_SIZE);
   nullTerminatePacketBuffer(packetBuffer,len);
}

bool hasPacket(BridgeUDP<> &udp) {
   return packetSize(udp);
}

int packetSize(BridgeUDP<> &udp) {
   return udp.parsePacket();
}

void nullTerminatePacketBuffer(char packetBuffer[BUFFER_SIZE], const int packetLength) {
   packetBuffer[packetLength] = NULL_TERMINATOR;
}

int packetLength(BridgeUDP<> &udp, char packetBuffer[BUFFER_SIZE], const int readableBufferSize) {
   return udp.read(packetBuffer, readableBufferSize);
}

void printPacket(BridgeMonitor<> &Monitor, char packetBuffer[BUFFER_SIZE]) {
   Monitor.print("Received: ");
   Monitor.println(packetBuffer);
}
// END FUNCTION DEFINITIONS