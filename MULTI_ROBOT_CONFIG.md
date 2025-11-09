# Multi-Robot UART Configuration Guide

## Overview
You have two robots with different UART pin configurations. The code now supports both via compile-time selection.

## Robot Configurations

### Robot 1 (Original Hardware)
- **UART:** USART2
- **Pins:** PD5 (TX), PD6 (RX)
- **Status:** Working original hardware

### Robot 2 (Reworked Hardware)
- **UART:** UART4
- **Pins:** PA0 (TX), PA1 (RX)
- **Status:** PD5/PD6 broken, reworked to PA0/PA1

## How to Compile for Each Robot

### In `src/drivetrain/Core/Inc/main.h`:

**For Robot 1 (USART2/PD5/PD6):**
```c
#define ROBOT_1_ORIGINAL  // USART2 on PD5/PD6 (original hardware)
// #define ROBOT_2_REWORK    // UART4 on PA0/PA1 (reworked hardware)
```

**For Robot 2 (UART4/PA0/PA1):**
```c
// #define ROBOT_1_ORIGINAL  // USART2 on PD5/PD6 (original hardware)
#define ROBOT_2_REWORK    // UART4 on PA0/PA1 (reworked hardware)
```

## Build Process

1. Open `main.h`
2. Uncomment the line for the target robot
3. Comment out the other robot's line
4. Build project
5. Flash to the corresponding robot

## Wiring Reference

### Robot 1:
```
ESP32 GPIO17 (TX) → STM32 PD6 (USART2_RX)
ESP32 GPIO16 (RX) → STM32 PD5 (USART2_TX)
GND               → GND
```

### Robot 2:
```
ESP32 GPIO17 (TX) → STM32 PA1 (UART4_RX)
ESP32 GPIO16 (RX) → STM32 PA0 (UART4_TX)
GND               → GND
```

## What the Code Does

The code uses preprocessor macros to select the correct UART:
- `huart_robot` = automatically resolves to `huart2` or `huart4`
- `MX_UART_Robot_Init()` = calls the correct init function
- All callbacks use `huart_robot` to work with either configuration

## Important Notes

- **Only one robot type can be active at compile time**
- If you forget to define either option, you'll get a compile error
- Both UART peripherals are initialized in the code, but only the selected one is used
- No runtime switching - must recompile for different robots

## Testing

After flashing:
1. Power cycle
2. Watch LED diagnostic sequence (green blinks)
3. Run `python wasd_teleop.py --robot <N>`
4. Verify ACK messages and green LED toggles

## Future: Runtime Detection

If you want to avoid recompiling, you could add:
- GPIO pin reading at startup to auto-detect robot type
- EEPROM/flash storage of robot ID
- Different firmware files per robot with clear naming (e.g., `robot1.bin`, `robot2.bin`)
