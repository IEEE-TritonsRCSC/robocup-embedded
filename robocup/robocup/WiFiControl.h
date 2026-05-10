/**
 * This file is to set up the WiFi for a UDP connection and packet parsing
 * The functions and defines make the WiFi features easier to use and read
 * To change the WiFi on the Arduino Uno Q, go to the Arduino App Lab > Settings > Select Network and enter WiFi credentials
 */

#pragma once

// BEGIN INCLUDES
#include <Arduino_RouterBridge.h>
// END INCLUDES

// BEGIN DEFINES
#define PORT 4210
#define BUFFER_SIZE 256
#define READABLE_BUFFER_SIZE BUFFER_SIZE - 1
#define NULL_TERMINATOR '\0'
// END DEFINES

// BEGIN FUNCTION DECLARATION
/**
 * @brief read an incoming UDP packet and print its contents to the Serial Monitor
 * @param udp the WiFi UDP object for the Router Bridge
 * @param Monitor the serial monitor which is the same as using `Serial` on a normal Arduino Uno
 * @param packetBuffer the buffer to hold the characters of the UDP packet's contents
 */
void parsePacket(
   BridgeUDP<> &udp, 
   BridgeMonitor<> &Monitor, 
   char packetBuffer[BUFFER_SIZE]
);
// END FUNCTION DECLARATION