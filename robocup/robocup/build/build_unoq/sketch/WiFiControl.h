#line 1 "C:\\Users\\bjsek\\Documents\\GitHub\\robocup-embedded\\robocup\\robocup\\WiFiControl.h"
/**
 * This file is to set up the WiFi for a UDP connection and packet parsing
 * The functions and defines make the WiFi features easier to use and read
 */

#pragma once

// BEGIN INCLUDES
#include "credentials.h"
#include <SPI.h>
#include <WiFi.h>
//#include <WiFiUdp.h>
// END INCLUDES

// BEGIN DEFINES
#define ENABLE_MULTICAST 0 // set this to 1 to enable WiFi multicast connection
#define PORT 10000 // WiFi port 10000 is yellow team, 11000 is blue team
#define PACKET_LENGTH 255 // max number of characters in an incoming UDP packet 
#define READABLE_PACKET_LENGTH PACKET_LENGTH - 1 // prevents buffer overflow if a packet is 255 chars long
#define NO_WIFI_SLEEP false // prevents the WiFi module from going into power-saving mode
/**
 * Set WiFi Tx Power to maximum (802.11b 20dBm)
 * Range is usually 8 to 78 (representing 2dBm to 19.5-20dBm)
 * Using WIFI_POWER_19_5dBm is the safest "max" constant.
 */
#define MAX_WIFI_PWR WIFI_POWER_19_5dBm
// END DEFINES

// BEGIN FUNCTION DECLARATION
/**
 * @brief attempt to connect to the WiFi
 */
void connectWiFi();
// END FUNCTION DECLARATION