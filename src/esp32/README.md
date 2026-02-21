# RoboCup Robot Control System

ESP32-based robot control system for RoboCup soccer robots with WiFi command interface, motor control, and kicker/dribbler hardware integration.

## Table of Contents
- [Overview](#overview)
- [System Flow](#system-flow)
  - [Startup Sequence](#1-startup-sequence)
  - [Main Loop Operation](#2-main-loop-operation)
  - [Command Processing Pipeline](#3-command-processing-pipeline)
- [Command Protocol](#command-protocol)
  - [Format](#format)
  - [Available Commands](#available-commands)
  - [Using tester.py](#using-testerpy)
- [Hardware Setup](#hardware-setup)
  - [Pin Configuration](#pin-configuration)
  - [Robot Physical Parameters](#robot-physical-parameters)
  - [Motor Command Format](#motor-command-format-uart)
- [System Behavior](#system-behavior)
  - [Movement Model](#movement-model)
  - [Kicker State Machine](#kicker-state-machine)
  - [Packet Processing Strategy](#packet-processing-strategy)
- [Configuration](#configuration)
- [Debug Output](#debug-output)

## Overview

This system receives commands over WiFi multicast UDP, parses text-based commands, and controls a 4-wheel omni-directional robot with kicker and dribbler capabilities. Motor commands are sent to an STM32 motor controller via UART.

## System Flow

### 1. Startup Sequence
1. ESP32 connects to WiFi network
2. Joins multicast group `239.42.42.42:11000`
3. Initializes UART connection to STM32 motor controller (115200 baud)
4. Initializes motor command buffer with header `0xCA 0xFE`
5. Configures GPIO pins (Kicker: GPIO 2, Solenoid: GPIO 15)
6. Performs initial kicker charge (100ms)
7. Prints "READY" and begins main loop

### 2. Main Loop Operation
1. **Check for UDP packets**: Reads all queued packets
2. **Select latest packet**: Only the most recent packet is processed (reduces jitter)
3. **Parse packet**: Character-by-character parsing into messages
4. **Execute command**: Calls appropriate executor function
5. **Update motor command**: Translates velocities to wheel speeds
6. **Send to STM32**: Transmits 11-byte motor command via UART
7. **Manage kicker**: Updates kicker state machine
8. **Repeat**

### 3. Command Processing Pipeline

```
UDP Packet → handleNewChar() → parseMsg() → parseCommand() → execute_*() → prepare_and_send_motor_command() → STM32
```

#### Example: Dash Command Flow
```
Packet: "1 dash 50.0 1.57"
  ↓
parseMsg() matches robot ID "1"
  ↓
parseCommand() identifies 'd' → dash
  ↓
execute_dash(50.0, 1.57)
  - acceleration = 0.006 * 50.0 = 0.3
  - vel_u += 0.3 * sin(1.57) ≈ 0.3
  - vel_v += 0.3 * cos(1.57) ≈ 0
  - vel_w = 0
  ↓
prepare_and_send_motor_command()
  - Translate (u,v,w) to 4 wheel velocities
  - Convert m/s to rad/s (divide by wheel radius)
  - Add rotational component
  - Convert wheel rad/s to motor RPM (rads -> RPM, apply reduction ratio)
  - Convert to int16
  - Pack into motor command buffer
  - Send 11 bytes to STM32
  - Apply velocity decay: vel_u *= 0.4, vel_v *= 0.4
```

## Command Protocol

Commands are sent via UDP multicast to `239.42.42.42:11000`:

### Format
```
<robot_id> <command> <parameters>
```

### Available Commands

| Command | Format | Description | Units |
|---------|--------|-------------|-------|
| `turn` | `1 turn <angular_speed>` | Set rotational velocity | rad/s |
| `dash` | `1 dash <power> <dir>` | Accelerate in direction | power (unitless), dir (radians) |
| `kick` | `1 kick` | Fire kicker solenoid | - |
| `skick` | `1 skick <power>` | Short kick (dribbler pulse) | 0-100 (ignored) |
| `catch` | `1 catch` | Activate dribbler | - |
| `stop` | `stop` | Emergency stop all motion | - |

### Using tester.py

The Python tester converts degrees to radians automatically:

```bash
python3 tester.py
```

```
Enter command: t 90        # Turn at 90 deg/s
Enter command: d 50 45     # Dash power 50 at 45 degrees
Enter command: k           # Kick
Enter command: c           # Catch (dribbler on)
Enter command: s 80        # Short kick pulse
Enter command: q           # Quit (sends stop)
```

## Hardware Setup

### Pin Configuration
- **TX**: GPIO 17 (UART to STM32)
- **RX**: GPIO 16 (UART from STM32)
- **Kicker**: GPIO 2 (LOW = fire, HIGH = off)
- **Solenoid**: GPIO 15 (HIGH = charging, LOW = off)

### Robot Physical Parameters
- **Wheel Radius**: 25mm
- **Robot Radius**: 86mm (center to wheel contact point)
- **Front Wheels**: 30° from forward axis
- **Back Wheels**: 60° from forward axis

### Motor Command Format (UART)
11-byte packet sent to STM32:
```
[0xCA][0xFE][FR_H][FR_L][BR_H][BR_L][BL_H][BL_L][FL_H][FL_L][DRIBBLER]
```
- **Header**: `0xCA 0xFE`
- **Wheel Speeds**: 4x int16 big-endian (motor RPM)
  - FR = Front-Right, BR = Back-Right, BL = Back-Left, FL = Front-Left
- **Dribbler**: 0x01 = on, 0x00 = off

## System Behavior

### Movement Model
- **Coordinates**: Robot-relative (u=sideways, v=forward, w=rotation)
- **Velocity Decay**: Linear velocities decay by 60% per command (multiply by 0.4)
- **Dash Acceleration**: `0.006 * power` added to velocity components
- **Turn**: Directly sets `w`, clears linear velocities
- **Motor RPM**: Wheel rad/s is converted to motor RPM using the reduction ratio before sending

### Kicker State Machine
1. **Charging** (100ms): Solenoid ON → charges capacitor
2. **Charged**: Ready to fire
3. **Firing** (100ms): Kicker pin LOW → discharges capacitor
4. **Cooldown** (5000ms): Safety wait before recharge
5. Loop back to Charging

### Packet Processing Strategy
Multiple UDP packets may queue up between loop iterations. The system:
- Reads ALL packets in the buffer
- Only processes the LAST packet received
- Discards intermediate packets
- **Why**: Reduces command latency and control jitter

## Configuration

Edit `globals.h` to customize:
- `ROBOT_NO`: Robot ID string (default "1")
- `MULTICAST_PORT`: UDP port (default 11000)
- `KICKER_CHARGING_TIME`: Capacitor charge time (default 100ms)
- `KICKING_TIME`: Solenoid fire duration (default 100ms)
- `WAIT_BEFORE_CHARGE_AGAIN`: Safety cooldown (default 5000ms)
- `DEBUG`: Enable/disable serial debug output

## Debug Output

When `DEBUG` is defined in `globals.h`, the PRINT() macro outputs:
- Packet size and receive timestamp
- Parsed commands with parameters
- Calculated wheel velocities
- Motor command values
- Command processing latency (microseconds)

Example output:
```
18 | Dashing with 50 power in 1.57 radians | (2300 -1500 -1500 2300 0) | (0.3 0.0 0.0)
1234
```
- `18`: Packet size in bytes
- Wheel speeds (RPM): FR, BR, BL, FL, Dribbler
- Velocities: (u, v, w)
- `1234`: Processing time in microseconds
