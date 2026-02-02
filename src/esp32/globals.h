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
#define MULTICAST_PORT 11000
#define MAX_PACKET_SIZE 512
#define MAX_BUFFER_SIZE 64
#define MAX_COMMAND_BUFFER 8
#define ROBOT_NO "1"
#define RELEVANT_FORMAT ROBOT_NO " %s %n"

// Send Motor Command over UART
#define DEFAULT_HEADER_BYTE_1 0xCA
#define DEFAULT_HEADER_BYTE_2 0xFE
#define MOTOR_CMD_HEADER_SIZE 2
#define MOTOR_CMD_TYPE_SIZE 1
#define MOTOR_CMD_PAYLOAD_OFFSET (MOTOR_CMD_HEADER_SIZE + MOTOR_CMD_TYPE_SIZE)
#define MOTOR_COMMAND_SIZE 12
#define DRIBBLER_MOTOR_INDEX 11
#define FRAME_TYPE_DRIVE_COMMAND 0x01
#define FRAME_TYPE_PID_UPDATE 0x02
#define FRAME_TYPE_HEADER_CONFIG 0x03
#define UART_DRIVE_PAYLOAD_SIZE 9
#define UART_PID_PAYLOAD_SIZE 13
#define UART_CONFIG_PAYLOAD_SIZE 3
#define HEADER_CHECKSUM_SEED 0xA5
#define TELEMETRY_HEADER_BYTE_1 0xFE
#define TELEMETRY_HEADER_BYTE_2 0xED
#define TELEMETRY_FRAME_SIZE 32
#define TELEMETRY_PORT 10001
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

extern std::array<uint8_t, MOTOR_COMMAND_SIZE> motor_command;
extern std::array<uint8_t, MOTOR_CMD_HEADER_SIZE> motor_cmd_headers;
extern HardwareSerial robotSerial;

#endif
