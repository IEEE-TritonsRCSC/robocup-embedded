/**
 * @file multi_motor.h
 * @brief Multi-motor functions using non-blocking commands
 */

#include "moteus.h"
#include "main.h"

extern FDCAN_HandleTypeDef hfdcan1;

#define NUM_MOTORS 5

/**
 * @brief FDCAN RX callback - add to main.c
 */
void HAL_FDCAN_RxFifo0Callback(
   FDCAN_HandleTypeDef *hfdcan,
   uint32_t RxFifo0ITs
);

/**
 * @brief Initialize all motors
 * @retval true  All motors initialized successfully
 * @retval false One or more motors failed to initialize
 */
bool motors_init(void);

/**
 * @brief Poll all motors, returns number of new responses
 */
int poll_all(void);

/**
 * @brief Wait for all motors to respond with timeout
 */
bool wait_all(uint32_t timeout_ms);

/**
 * @brief Example: Query all motors
 */
void query_all(void);

/**
 * @brief Example: Control loop sending different torques to each motor
 *
 * Call from timer interrupt at your control rate (e.g., 1kHz).
 */
void control_loop_tick(float* target_positions);

/**
 * @brief Stop all motors
 */
void stop_all(void);

/**
 * @brief Example main
 */
void example_main(void);
