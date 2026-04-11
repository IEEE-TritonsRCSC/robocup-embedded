/**
 * @file multi_motor.c
 * @brief Multi-motor example using non-blocking commands
 *
 * This example demonstrates:
 * - Managing multiple motors
 * - Non-blocking commands (Begin*)
 * - Polling for responses
 * - High-frequency control loop pattern
 *
 * Prerequisites:
 * - FDCAN configured in CubeMX (see README for timing settings)
 * - FDCAN global interrupt enabled
 * - MOTEUS_MAX_MOTORS >= NUM_MOTORS
 */

#include "multi_motor.h"

static moteus_motor_t* motors[NUM_MOTORS];

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    (void)RxFifo0ITs;
    moteus_fdcan_rx_callback(hfdcan);
}

bool motors_init(void)
{
    moteus_can_init(&hfdcan1);

    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i] = moteus_init(&hfdcan1, i + 1);  // IDs 1, 2, 3
        if (!motors[i]) {
            printf("Failed to init motor %d\n", i + 1);
            return false;
        }
    }
    return true;
}

int poll_all(void)
{
    int received = 0;
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (moteus_poll(motors[i])) {
            received++;
        }
    }
    return received;
}

bool wait_all(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    int received = 0;

    while (received < NUM_MOTORS) {
        if (HAL_GetTick() - start > timeout_ms) {
            return false;
        }
        received += poll_all();
    }
    return true;
}

void query_all(void)
{
    // Send queries (non-blocking)
    for (int i = 0; i < NUM_MOTORS; i++) {
        moteus_begin_query(motors[i]);
    }

    // Wait for responses
    if (wait_all(50)) {
        for (int i = 0; i < NUM_MOTORS; i++) {
            printf("Motor %d: pos=%.3f vel=%.3f\n",
                   i + 1,
                   motors[i]->result.position,
                   motors[i]->result.velocity);
        }
    }
}

void control_loop_tick(float* target_positions)
{
    // Check for responses from previous cycle
    poll_all();

    // Compute and send torque for each motor
    for (int i = 0; i < NUM_MOTORS; i++) {
        float pos = motors[i]->result.position;
        float vel = motors[i]->result.velocity;

        // Simple PD control (each motor may have different target)
        float error = target_positions[i] - pos;
        float torque = 10.0f * error - 0.5f * vel;

        // Clamp
        if (torque > 2.0f) torque = 2.0f;
        if (torque < -2.0f) torque = -2.0f;

        moteus_begin_torque(motors[i], torque);
    }
}

void stop_all(void)
{
    for (int i = 0; i < NUM_MOTORS; i++) {
        moteus_begin_stop(motors[i]);
    }
    wait_all(50);
}

void example_main(void)
{
    if (!motors_init()) {
        return;
    }

    query_all();

    // Run control loop for 5 seconds
    float targets[NUM_MOTORS] = {0.0f}; // init targets to 0.0f
    uint32_t start = HAL_GetTick();

    while (HAL_GetTick() - start < 5000) {
        control_loop_tick(targets);
        HAL_Delay(1);  // 1kHz
    }

    stop_all();
}
