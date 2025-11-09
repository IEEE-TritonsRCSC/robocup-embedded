# UART4 Migration Summary

## Changes Made

Updated STM32 firmware to use **UART4 on PA0/PA1** instead of USART2 on PD5/PD6.

### Reason
Physical pins PD5/PD6 are broken, so we switched to UART4 which is available on PA0/PA1.

### Pin Mapping
- **PA0** = UART4_TX (connects to ESP32 GPIO16 RX)
- **PA1** = UART4_RX (connects to ESP32 GPIO17 TX)
- **GND** = Common ground

### Files Modified

1. **src/drivetrain/Core/Inc/usart.h**
   - Added `extern UART_HandleTypeDef huart4;`
   - Added `void MX_UART4_Init(void);`

2. **src/drivetrain/Core/Src/usart.c**
   - Added `UART_HandleTypeDef huart4;`
   - Added `MX_UART4_Init()` function
   - Updated `HAL_UART_MspInit()` to configure PA0/PA1 with AF8_UART4
   - Updated `HAL_UART_MspDeInit()` for cleanup

3. **src/drivetrain/Core/Src/main.c**
   - Changed all `&huart2` references to `&huart4` in UART callbacks
   - Added `MX_UART4_Init()` call in initialization sequence
   - Updated `HAL_UART_Receive_IT(&huart4, ...)` calls
   - Updated `HAL_UART_Transmit(&huart4, ...)` for ACK

4. **src/drivetrain/Core/Src/stm32f4xx_it.c**
   - Added `extern UART_HandleTypeDef huart4;`
   - Added `UART4_IRQHandler()` interrupt handler

### Configuration Details
- **Baud Rate:** 115200
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None
- **GPIO Alternate Function:** AF8 (UART4)
- **Interrupt Priority:** 0, 0

### Testing
After flashing:
1. Power cycle both ESP32 and STM32
2. Watch for LED diagnostic sequence on STM32 (green blinks)
3. Run teleop: `python wasd_teleop.py --robot 1`
4. ESP32 should show: "STM32 ACK received: 01"
5. STM32 green LED should toggle on each command

### Physical Wiring
```
ESP32 GPIO17 (TX) ──→ STM32 PA1 (UART4_RX)
ESP32 GPIO16 (RX) ←── STM32 PA0 (UART4_TX)
ESP32 GND         ──── STM32 GND
```

### Frame Format (unchanged)
```
ESP32 → STM32:  CA FE [8 bytes wheel speeds] [1 byte dribbler]
STM32 → ESP32:  AC CE 01 (acknowledgment)
```
