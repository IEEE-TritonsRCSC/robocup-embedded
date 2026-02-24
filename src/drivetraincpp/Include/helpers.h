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
#define MOTOR1_GAINS {0.3f, 0.0f, 0.0f}
#define MOTOR2_GAINS {0.3f, 0.0f, 0.0f}
#define MOTOR3_GAINS {0.2f, 0.0f, 0.0f}
#define MOTOR4_GAINS {0.2f, 0.0f, 0.0f}
#define TIMEOUT_DELAY 200

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

/**
 * @brief Update the dribbler speed based on the dribble flag state.
 *
 * @param dribble_flag Flag indicating whether dribbler should be active.
 * @param dribble_speed Pointer to the dribbler speed output value.
 */
void updateDribblerSpeedFromFlag(int dribble_flag, int16_t* dribble_speed);
/**
 * @brief Apply safety timeout behavior to target wheel speeds.
 *
 * @param timeout Timeout flag/counter indicating communication loss.
 * @param targetSpeeds Target speed array to modify when timed out.
 */
void applySafetyTimeoutToTargetSpeeds(int timeout, volatile float *targetSpeeds);
/**
 * @brief Run one PID update step for all motors.
 *
 * @param motor_pids Array of PID state for each motor.
 * @param targetSpeeds Target speed array for each motor.
 * @param speed_data Current speed feedback array for each motor.
 */
void updateMotorPidLoop(PID_Data *motor_pids, volatile float *targetSpeeds, volatile float *speed_data);
/**
 * @brief Initialize motor GPIO pin mappings.
 *
 * @param motorPins Array to receive motor GPIO pin identifiers.
 */
void setupMotors(uint16_t* motorPins);
/**
 * @brief Turn off all status LEDs.
 */
void turnLEDsOff();

/**
 * @brief Pack motor speed commands into CAN payloads and transmit them.
 *
 * @param state Shared drivetrain state.
 * @param speedCommands Array of 5 motor speed commands (motors 1-4, dribbler 5).
 */
void setMotorSpeeds(DrivetrainState *state, int16_t speedCommands[5]);

// Helper routines for HAL callbacks (use shared state).
/**
 * @brief Handle CAN RX FIFO0 callback logic using shared state.
 *
 * @param state Shared drivetrain state.
 * @param hcan CAN handle associated with the interrupt.
 */
void handleCanRxFifo0(DrivetrainState *state, CAN_HandleTypeDef *hcan);
/**
 * @brief Handle UART RX complete callback logic using shared state.
 *
 * @param state Shared drivetrain state.
 * @param huart UART handle associated with the interrupt.
 */
void handleUartRxComplete(DrivetrainState *state, UART_HandleTypeDef *huart);

/**
 * @brief Enable the power interface clock and set voltage scaling for max speed.
 */
void configureOutputVoltage();

/**
 * @brief Populate the RCC oscillator config struct for the HSE + PLL setup.
 *
 * @param RCC_OscInitStruct Target oscillator config to fill in.
 */
void initializeRCCOscillator(RCC_OscInitTypeDef *RCC_OscInitStruct);

/**
 * @brief Populate the clock tree config struct for SYSCLK/HCLK/PCLK dividers.
 *
 * @param RCC_ClkInitStruct Target clock config to fill in.
 */
void initializeClocks(RCC_ClkInitTypeDef *RCC_ClkInitStruct);

/**
 * @brief Apply oscillator configuration and halt on failure.
 *
 * @param RCC_OscInitStruct Oscillator configuration to apply.
 */
void configureOscillatorOrDie(RCC_OscInitTypeDef *RCC_OscInitStruct);

/**
 * @brief Apply clock tree configuration and halt on failure.
 *
 * @param RCC_ClkInitStruct Clock configuration to apply.
 */
void configureClockTreeOrDie(RCC_ClkInitTypeDef *RCC_ClkInitStruct);

/**
 * @brief Initialize and apply the oscillator configuration.
 *
 * @param RCC_OscInitStruct Oscillator configuration to initialize and apply.
 */
void initializeAndConfigureOscillatorOrDie(RCC_OscInitTypeDef *RCC_OscInitStruct);

/**
 * @brief Initialize and apply the clock tree configuration.
 *
 * @param RCC_ClkInitStruct Clock configuration to initialize and apply.
 */
void initializeAndConfigureClockTreeOrDie(RCC_ClkInitTypeDef *RCC_ClkInitStruct);

/**
 * @brief Initialize and apply oscillator and clock tree configurations.
 *
 * @param RCC_OscInitStruct Oscillator configuration to initialize and apply.
 * @param RCC_ClkInitStruct Clock configuration to initialize and apply.
 */
void initializeAndConfigureOscillatorAndClockTreeOrDie(RCC_OscInitTypeDef *RCC_OscInitStruct, RCC_ClkInitTypeDef *RCC_ClkInitStruct);

/**
 * @brief Initialize board peripherals (GPIO/CAN/UART/timers).
 *
 * Called during startup to bring up the hardware drivers used by drivetrain.
 */
void initializePeripherals();

/**
 * @brief HAL callback for CAN RX FIFO0 message pending.
 *
 * @param hcan CAN handle that triggered the callback.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

/**
 * @brief HAL callback for UART receive complete.
 *
 * @param huart UART handle that triggered the callback.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief Fill speed command array from PID outputs and dribbler speed.
 *
 * @param state Shared drivetrain state containing PID outputs and dribbler speed.
 * @param speedCommands Output array sized NUM_MOTORS (wheels + dribbler).
 */
void initializeSpeedCommands(DrivetrainState *state, int16_t *speedCommands);

// System hooks.
#ifdef __cplusplus
    extern "C" {
#endif
/**
 * @brief Configure system clocks and PLL.
 */
void SystemClock_Config(void);
/**
 * @brief HAL timer period elapsed callback.
 *
 * @param htim Timer handle that triggered the callback.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
/**
 * @brief Error handler for unrecoverable faults.
 */
void Error_Handler(void);
#ifdef __cplusplus
}
#endif
