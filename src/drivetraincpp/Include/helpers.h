#pragma once

#include <stdint.h>
#include "PID_Data.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"
#include "CanHeader.h"

// Protocol constants shared with helper callbacks.
#define UART_RX_BUFFER_SIZE 9
#define UART_TX_BUFFER_SIZE 12
#define HEADER_BYTE_1 0xCA
#define HEADER_BYTE_2 0xFE
#define DRIBBLE_ON 0x01
#define FRAME_LENGTH 8
#define NUM_WHEELS 4
#define NUM_MOTORS 5
#define MAX_OUTPUT 9999.0f
#define INTEGRAL_LIMIT 1000.0f
#define DEADBAND 20.0f
#define CANTXHEADER1_STDID 0x200
#define CANTXHEADER2_STDID 0x1FF

struct DrivetrainState {
    // Hardware handles (assigned in main after init).
    CAN_HandleTypeDef *can;
    UART_HandleTypeDef *uart;

    // CAN headers and buffers.
    CAN_TxHeaderTypeDef canTxHeader1;
    CAN_TxHeaderTypeDef canTxHeader2;
    CanHeader canHeader1;
    CanHeader canHeader2;
    CAN_RxHeaderTypeDef canRxHeader;
    uint32_t canTxMailbox;
    uint8_t canTxData[FRAME_LENGTH];
    uint8_t can2TxData[FRAME_LENGTH];
    uint8_t canRxData[FRAME_LENGTH];

    // PID feedback variables.
    volatile uint8_t motor_idx;
    volatile uint16_t angle_data[NUM_WHEELS];
    volatile float speed_data[NUM_WHEELS];
    volatile float torque_current_data[NUM_WHEELS];
    volatile float targetSpeeds[NUM_WHEELS];

    // UART buffers and flags.
    uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
    uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
    volatile uint8_t rx_byte;
    volatile int header1_flag;
    volatile int header2_flag;
    volatile int timeout;
    volatile int dribble_flag;
    int16_t dribble_speed;

    // Motors + PID state.
    uint16_t motorPins[NUM_MOTORS];
    PID_Gains motor1_gains;
    PID_Gains motor2_gains;
    PID_Gains motor3_gains;
    PID_Gains motor4_gains;
    PID_Data motor1PID;
    PID_Data motor2PID;
    PID_Data motor3PID;
    PID_Data motor4PID;
    PID_Data motor_pids[NUM_WHEELS];

    DrivetrainState();
};

void updateDribblerSpeedFromFlag(int dribble_flag, int16_t* dribble_speed);
void applySafetyTimeoutToTargetSpeeds(int timeout, volatile float *targetSpeeds);
void updateMotorPidLoop(PID_Data *motor_pids, volatile float *targetSpeeds, volatile float *speed_data);
void setupMotors(uint16_t* motorPins);
void turnLEDsOff();

void setMotorSpeeds(DrivetrainState *state,
        int16_t ms1, int16_t ms2, int16_t ms3, int16_t ms4, int16_t msg5);

// Helper routines for HAL callbacks (use shared state).
void handleCanRxFifo0(DrivetrainState *state, CAN_HandleTypeDef *hcan);
void handleUartRxComplete(DrivetrainState *state, UART_HandleTypeDef *huart);

// System hooks.
#ifdef __cplusplus
extern "C" {
#endif
void SystemClock_Config(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void Error_Handler(void);
#ifdef __cplusplus
}
#endif
