#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\WiFiControl.h"
/**
 * This file is to set up the WiFi for a UDP connection and packet parsing
 * The functions and defines make the WiFi features easier to use and read
 * To change the WiFi on the Arduino Uno Q, go to the Arduino App Lab > Settings > Select Network and enter WiFi credentials
 */

#pragma once

// BEGIN INCLUDES
#include <Arduino_RouterBridge.h>
// END INCLUDES

// BEGIN CONSTEXPR
constexpr unsigned int PORT = 4210;
constexpr unsigned int BUFFER_SIZE = 256;
constexpr unsigned int READABLE_BUFFER_SIZE = BUFFER_SIZE - 1;
constexpr char NULL_TERMINATOR = '\0';
// END CONSTEXPR

// BEGIN FUNCTION DECLARATION
/**
 * @brief read an incoming UDP packet and print its contents to the Serial Monitor
 * @param udp the WiFi UDP object for the Router Bridge
 * @param Monitor the serial monitor which is the same as using `Serial` on a normal Arduino Uno
 * @param packetBuffer holds the characters of the UDP packet's contents
 */
void parsePacket(
   BridgeUDP<> &udp, 
   BridgeMonitor<> &Monitor, 
   char packetBuffer[BUFFER_SIZE]
);

/**
 * @brief read the data in a UDP packet into the packet buffer and return the length of the packet
 * @param udp WiFi UDP object
 * @param packetBuffer holds the UDP packet data
 * @param readableBufferSize the size of the buffer - 1
 * @return the length of the data in the UDP packet
 * @note the readable buffer size is 1 less than the buffer size to prevent a buffer overflow if the packet is as long as the buffer. We need one character to put the null terminator character at the end
 */
int packetLength(
   BridgeUDP<> &udp, 
   char packetBuffer[BUFFER_SIZE], 
   const int readableBufferSize
);

/**
 * @brief check if there is an available UDP packet to read
 * @param udp WiFi UDP object
 * @return is udp.parsePacket() a non-zero int?
 * @retval true udp.parsePacket() returned a non-zero int. there is data in the UDP packet
 * @retval false udp.parsePacket() returned zero. The UDP packet is empty
 */
bool hasPacket(BridgeUDP<> &udp);

/**
 * @brief put a null terminator character at the end of the UDP packet's data inside of the packet buffer
 * @param packetBuffer holds the UDP packet data
 * @param packetLength length of the UDP packet data inside of the packet buffer
 */
void nullTerminatePacketBuffer(
   char packetBuffer[BUFFER_SIZE], 
   const int packetLength
);

/**
 * @brief Checks for the presence of a UDP packet, and returns the size
 * @param udp WiFi UDP object for the Router Bridge
 * @return the size of the UDP packet
 * @note the end of the packet does not have a null terminator char like a normal string
 */
int packetSize(BridgeUDP<> &udp);

/**
 * @brief print a packet buffer to the Serial Monitor
 * @param Monitor Serial Monitor object
 * @param packetBuffer holds the characters of the UDP packet's contents
 */
void printPacket(BridgeMonitor<> &Monitor, char packetBuffer[BUFFER_SIZE]);
// END FUNCTION DECLARATION