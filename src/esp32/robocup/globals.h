#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Robot Identity (Matches your Source [1])
#define ROBOT_ID "1"
#define RELEVANT_FORMAT ROBOT_ID " %s %n"

// Hardware Mapping (Matches your Source [2])
#define UART_BAUD 115200
#define TX_PIN 17
#define RX_PIN 16

// Protocol Sizes
#define NUM_MOTORS 5 
#define TEL_PACKET_SIZE 31

// Packed Structure for STM32 Communication
struct __attribute__((packed)) CommandPacket_t {
  float velocities[NUM_MOTORS]; // 20 bytes
  uint8_t kick;                 // 1 byte
  uint8_t chip;                 // 1 byte
};

// Global Shared Variables
extern CommandPacket_t current_cmd;
extern HardwareSerial robotSerial;
extern WiFiUDP UDP;

// --- FUNCTION PROTOTYPES (Fixes 'not declared' errors) ---
void parseMsg(char *msg);
void calculateKinematics(float vx, float vy, float wz);

#endif