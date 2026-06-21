/**
 * This file defines the functions of WiFiControls.h
 * The functions makes WiFi features easier to use and read
 */

// BEGIN INCLUDES
#include "WiFiControl.h"
// END INCLUDES

// BEGIN FUNCTION DEFINITIONS
void readPacketIntoBuffer(
   UDP &udp, 
   char packetBuffer[BUFFER_SIZE]) 
{
   int len = packetLength(udp,packetBuffer,READABLE_BUFFER_SIZE);
   nullTerminatePacketBuffer(packetBuffer,len);
}

int hasPacket(UDP &udp) {
   return udp.parsePacket();
}

void nullTerminatePacketBuffer(char packetBuffer[BUFFER_SIZE], const int packetLength) {
   packetBuffer[packetLength] = NULL_TERMINATOR;
}

int packetLength(UDP &udp, char packetBuffer[BUFFER_SIZE], const int readableBufferSize) {
   return udp.read(packetBuffer, readableBufferSize);
}

void printPacket(char packetBuffer[BUFFER_SIZE]) {
   Serial.print("Received: ");
   Serial.println(packetBuffer);
}
// END FUNCTION DEFINITIONS