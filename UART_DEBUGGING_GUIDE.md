# UART Communication Debugging Guide

## Problem Analysis
The STM32 was not responding to ESP32 commands. Root cause: **one-way communication with no feedback mechanism**.

## Changes Made

### 1. STM32 (main.c)
**Added acknowledgment transmission:**
- When STM32 successfully receives and parses a valid 11-byte frame (0xCA 0xFE + 9 bytes), it now sends back:
  ```
  0xAC 0xCE 0x01  (ACK header + success byte)
  ```
- This happens in `HAL_UART_RxCpltCallback()` after updating targetSpeeds
- Green LED still toggles on valid message

### 2. ESP32 (robocup.ino)
**Added ACK monitoring:**
- After transmitting to STM32, ESP32 now waits up to 50ms for acknowledgment
- If ACK received: prints "STM32 ACK received" and blinks LED
- If no ACK: prints "WARNING: No ACK from STM32!"
- Enhanced TX debug: prints full hex dump of transmitted frame

## Debugging Steps

### Step 1: Verify ESP32 Receives UDP
1. Open Arduino Serial Monitor (115200 baud) with ESP32 connected
2. Run teleop: `python wasd_teleop.py --gui`
3. Press keys (W/S/A/D)
4. **Expected output:**
   ```
   command: dash
   pow: 5.0, angle: 0.0
   TX to STM32: CA FE 00 64 00 C8 FF CE 00 00 01
   ```
5. **If you DON'T see this:** Check WiFi connection, UDP multicast, robot number match

### Step 2: Verify ESP32 Transmits UART
1. Check TX pin (GPIO 17) with logic analyzer or oscilloscope
2. Should see 115200 baud UART frames when commands sent
3. **If no signal:** Check `robotSerial.begin()` initialization

### Step 3: Verify STM32 Receives UART
1. Flash updated STM32 firmware with ACK support
2. Watch ESP32 serial monitor for:
   ```
   STM32 ACK received: 01
   ```
3. **If "WARNING: No ACK from STM32!":**
   - Check physical wiring: ESP32 TX (GPIO17) → STM32 RX (PA3/USART2)
   - Check GND common between boards
   - Verify STM32 USART2 initialized (115200 8N1)
   - Check if STM32 green LED toggles (visible feedback)

### Step 4: Verify STM32 Processes Commands
1. Watch STM32 green LED - should toggle on each valid frame
2. Connect CAN analyzer to see motor commands on 0x200
3. Check motor encoders respond via CAN feedback (0x201-0x204)

## Common Issues & Solutions

### Issue: ESP32 shows "WARNING: No ACK"
**Possible causes:**
- **Wiring:** ESP32 TX → STM32 RX not connected, or wrong pin
- **Baud mismatch:** Both must be 115200
- **STM32 not running:** Check power, reset, flash success
- **UART not initialized:** Verify `MX_USART2_UART_Init()` called

**Quick test:**
- Temporarily loop STM32 USART2 TX→RX physically
- ESP32 should receive its own frames back

### Issue: ESP32 never prints "TX to STM32"
**Possible causes:**
- UDP commands not arriving (check Step 1)
- Robot number mismatch (`ROBOT_NO` in ESP32 vs `--robot` in teleop)
- Command parsing failure (check `processCommand()`)

### Issue: STM32 receives but motors don't move
**Possible causes:**
- PID not initialized (check `pid_init()` calls)
- CAN bus not working (check `HAL_CAN_Start()`)
- Motor power supply off
- Target speeds clipped by timeout (increase timeout or check streaming rate)

## Frame Formats Reference

### ESP32 → STM32 (Command Frame)
```
Byte 0:    0xCA       Header 1
Byte 1:    0xFE       Header 2
Bytes 2-3: s0 (int16) Front Right wheel speed
Bytes 4-5: s1 (int16) Back Right wheel speed
Bytes 6-7: s2 (int16) Back Left wheel speed
Bytes 8-9: s3 (int16) Front Left wheel speed
Byte 10:   0x01/0x00  Dribbler on/off
```
**Total: 11 bytes**

### STM32 → ESP32 (Acknowledgment)
```
Byte 0: 0xAC    ACK header 1
Byte 1: 0xCE    ACK header 2
Byte 2: 0x01    Status (0x01 = success)
```
**Total: 3 bytes**

## Advanced Debugging

### Use Logic Analyzer
1. Connect to ESP32 TX (GPIO17) and STM32 TX (PA2)
2. Set decoder to UART 115200 8N1
3. Verify:
   - ESP32 sends complete 11-byte frames
   - STM32 responds with 3-byte ACK within ~10ms
   - Frame structure matches spec above

### Add More Verbose STM32 Feedback
If you need richer debugging info from STM32, modify the ACK to include:
```c
uint8_t ack_msg[7];
ack_msg[0] = 0xAC;
ack_msg[1] = 0xCE;
ack_msg[2] = 0x01; // Success
ack_msg[3] = (uint8_t)(targetSpeeds[0] >> 8); // Echo back FR speed high
ack_msg[4] = (uint8_t)(targetSpeeds[0]);      // FR speed low
ack_msg[5] = dribble_flag;
ack_msg[6] = (uint8_t)(timeout);
HAL_UART_Transmit(&huart2, ack_msg, 7, 10);
```

This lets ESP32 verify STM32 parsed values correctly.

## Testing Checklist

- [ ] ESP32 connects to WiFi and joins multicast group
- [ ] ESP32 receives UDP packets from teleop
- [ ] ESP32 parses commands correctly (dash/turn/kick)
- [ ] ESP32 transmits 11-byte UART frames
- [ ] STM32 receives UART interrupt fires
- [ ] STM32 parses 0xCA 0xFE header correctly
- [ ] STM32 updates targetSpeeds array
- [ ] STM32 sends ACK back to ESP32
- [ ] ESP32 receives and logs ACK
- [ ] STM32 green LED toggles on valid frames
- [ ] STM32 PID loop runs with new targets
- [ ] CAN bus transmits motor commands (0x200)
- [ ] Motors respond with feedback (0x201-0x204)
- [ ] Wheels physically move

## Useful Commands

```bash
# Run teleop with debug
cd src/TestServer
python wasd_teleop.py --gui --robot 1

# Monitor ESP32 serial output
# (Arduino IDE Serial Monitor at 115200 baud)

# Check for compile errors on STM32
# (STM32CubeIDE: Project → Build All)
```

## Next Steps
If still not working after following this guide:
1. Capture logic analyzer trace and share
2. Check ESP32 serial output and share full log
3. Verify physical connections with multimeter continuity test
4. Try loopback test (ESP32 TX→RX shorted, should see self-echo)
