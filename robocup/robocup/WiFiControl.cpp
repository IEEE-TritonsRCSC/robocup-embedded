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
   int packetSize = udp.parsePacket();
   if (packetSize) {
      int len = udp.read(packetBuffer, READABLE_BUFFER_SIZE);
      packetBuffer[len] = NULL_TERMINATOR;
      Monitor.print("Received: ");
      Monitor.println(packetBuffer);
   }
}
// END FUNCTION DEFINITIONS