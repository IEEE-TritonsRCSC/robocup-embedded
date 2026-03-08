#ifndef GLOBALS_H
#define GLOBALS_H

#include <array>
#include "Arduino.h"

#define DEBUG

#ifdef DEBUG

template<typename T>
void PRINT(const T& value) {
    Serial.print(value);
}

template<typename T, typename... Args>
void PRINT(const T& first, const Args&... rest) {
    Serial.print(first);
    PRINT(rest...);
}

#else

template<typename... Args>
void PRINT(const Args&...) {}

#endif

// Receive Messages over multicast
#define MULTICAST_PORT 10000
#define MAX_PACKET_SIZE 512
#define MAX_BUFFER_SIZE 64
#define MAX_COMMAND_BUFFER 8
#define ROBOT_NO "1"
#define RELEVANT_FORMAT ROBOT_NO " %s %n"

// Send Motor Command over UART
#define MOTOR_CMD_HEADER_SIZE 2
#define MOTOR_COMMAND_SIZE 11
#define DRIBBLER_MOTOR_INDEX 10
#define UART_HEADER_1 0xCA
#define UART_HEADER_2_RUNTIME 0xFE
#define UART_HEADER_2_PID 0xEE
#define UART_PID_PAYLOAD_SIZE 7
#define UART_PID_PACKET_SIZE (MOTOR_CMD_HEADER_SIZE + UART_PID_PAYLOAD_SIZE)
#define TX_PIN 17
#define RX_PIN 16

#define KICKER_PIN 2
#define SOLENOID_PIN 15
#define KICKER_CHARGING_TIME 100  // ms
#define KICKING_TIME 100  // ms
#define WAIT_BEFORE_CHARGE_AGAIN 5000  // 5 seconds

extern unsigned long packet_time;
extern int size;
extern char buffer[MAX_BUFFER_SIZE];
extern char cmd_buffer[MAX_COMMAND_BUFFER];

extern bool kicker_charged;
extern bool charging_kicker;
extern unsigned long start_charge_time;
extern unsigned long last_kick_time;

extern std::array<uint8_t, 11> motor_command;
extern HardwareSerial robotSerial;

#endif
