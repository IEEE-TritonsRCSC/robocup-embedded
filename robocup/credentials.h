/**
 * @file credentials.h
 * @brief Network configuration for the robot controller.
 *
 * This file contains local Wi-Fi and UDP settings used by the firmware.
 * Keep it out of source control if it contains environment-specific secrets.
 */
#pragma once

#define WIFI_SSID "wlan3"
#define WIFI_PASSWORD "a1b2c3d4"
#define UDP_PORT 10000
#define LOCAL_IP_ADDRESS IPAddress(192, 168, 68, 50)
#define GATEWAY_IP_ADDRESS IPAddress(192, 168, 68, 1)
#define SUBNET_MASK IPAddress(255, 255, 255, 0)
