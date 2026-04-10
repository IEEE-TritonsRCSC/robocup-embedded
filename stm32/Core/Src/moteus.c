/**
 * @file moteus.c
 * @brief Moteus motor controller library implementation
 *
 * Main API implementation for controlling Moteus brushless motor controllers.
 */

#include "moteus.h"
#include <math.h>
#include <string.h>

/* ============================================================================
 * Internal Declarations
 * ============================================================================ */

/* Defined in moteus_can.c */
HAL_StatusTypeDef moteus_can_transmit(FDCAN_HandleTypeDef* hfdcan,
                                       const moteus_can_frame_t* frame);

/* ============================================================================
 * Motor Instance Pool
 * ============================================================================ */

static moteus_motor_t motor_pool[MOTEUS_MAX_MOTORS];
static moteus_motor_t* motor_registry[128] = {NULL};  /* Index by motor ID */
static uint8_t motors_allocated = 0;

/* ============================================================================
 * Initialization
 * ============================================================================ */

moteus_motor_t* moteus_init(FDCAN_HandleTypeDef* hfdcan, uint8_t motor_id)
{
    if (hfdcan == NULL || motor_id == 0 || motor_id > 127) {
        return NULL;
    }

    /* Check if already registered */
    if (motor_registry[motor_id] != NULL) {
        return motor_registry[motor_id];  /* Return existing instance */
    }

    /* Allocate from pool */
    if (motors_allocated >= MOTEUS_MAX_MOTORS) {
        return NULL;  /* Pool exhausted */
    }

    moteus_motor_t* motor = &motor_pool[motors_allocated++];

    /* Initialize motor instance */
    memset(motor, 0, sizeof(moteus_motor_t));
    motor->hfdcan = hfdcan;
    motor->id = motor_id;
    motor->registered = true;

    /* Set default resolutions */
    moteus_query_resolution_t default_query = MOTEUS_QUERY_RESOLUTION_DEFAULT;
    motor->query_res = default_query;

    moteus_position_resolution_t default_cmd = MOTEUS_POSITION_RESOLUTION_DEFAULT;
    motor->cmd_res = default_cmd;

    /* Initialize result with NaN */
    motor->result.mode = 0;
    motor->result.fault = 0;
    motor->result.position = NAN;
    motor->result.velocity = NAN;
    motor->result.torque = NAN;
    motor->result.q_current = NAN;
    motor->result.d_current = NAN;
    motor->result.voltage = NAN;
    motor->result.temperature = NAN;
    motor->result.motor_temperature = NAN;
    motor->result.power = NAN;
    motor->result.abs_position = NAN;
    motor->result.rezero_state = 0;
    motor->result.trajectory_complete = false;
    motor->response_received = false;
    motor->timeout_ms = MOTEUS_DEFAULT_TIMEOUT_MS;

    /* Register in lookup table */
    motor_registry[motor_id] = motor;

    return motor;
}

void moteus_deinit(moteus_motor_t* motor)
{
    if (motor == NULL) return;

    /* Unregister */
    if (motor->id <= 127 && motor_registry[motor->id] == motor) {
        motor_registry[motor->id] = NULL;
    }

    motor->registered = false;
    /* Note: We don't actually free from the pool - just unregister */
}

/* ============================================================================
 * Configuration
 * ============================================================================ */

void moteus_set_query_resolution(moteus_motor_t* motor,
                                  const moteus_query_resolution_t* res)
{
    if (motor == NULL || res == NULL) return;
    motor->query_res = *res;
}

void moteus_set_command_resolution(moteus_motor_t* motor,
                                    const moteus_position_resolution_t* res)
{
    if (motor == NULL || res == NULL) return;
    motor->cmd_res = *res;
}

void moteus_set_response_callback(moteus_motor_t* motor,
                                   moteus_response_cb_t callback,
                                   void* user_data)
{
    if (motor == NULL) return;
    motor->response_cb = callback;
    motor->response_user_data = user_data;
}

void moteus_set_error_callback(moteus_motor_t* motor,
                                moteus_error_cb_t callback,
                                void* user_data)
{
    if (motor == NULL) return;
    motor->error_cb = callback;
    motor->error_user_data = user_data;
}

/* ============================================================================
 * Commands
 * ============================================================================ */

/**
 * @brief Wait for response with timeout (internal helper)
 * @return true if response received, false on timeout
 *
 * Waits for response_received flag to be set by interrupt callback.
 */
static bool wait_for_response(moteus_motor_t* motor)
{
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < motor->timeout_ms) {
        /* Check if response received by interrupt handler */
        if (motor->response_received) {
            motor->response_received = false;
            return true;
        }
    }

    return false;
}

/**
 * @brief Check result for errors and invoke callbacks (internal helper)
 * @return MOTEUS_OK if no error, error code otherwise
 *
 * Checks response for fault/timeout modes, updates error tracking,
 * and invokes error callback if registered.
 */
static int check_result_for_errors(moteus_motor_t* motor)
{
    const moteus_result_t* result = &motor->result;

    /* Check for fault mode */
    if (result->mode == MOTEUS_MODE_FAULT) {
        motor->last_error = MOTEUS_ERR_MOTOR_FAULT;
        motor->last_fault = result->fault;
        motor->fault_count++;

        if (motor->error_cb != NULL) {
            motor->error_cb(motor, MOTEUS_ERR_MOTOR_FAULT, motor->error_user_data);
        }
        return MOTEUS_ERR_MOTOR_FAULT;
    }

    /* Check for timeout mode */
    if (result->mode == MOTEUS_MODE_TIMEOUT) {
        motor->last_error = MOTEUS_ERR_MOTOR_TIMEOUT;
        motor->timeout_count++;

        if (motor->error_cb != NULL) {
            motor->error_cb(motor, MOTEUS_ERR_MOTOR_TIMEOUT, motor->error_user_data);
        }
        return MOTEUS_ERR_MOTOR_TIMEOUT;
    }

    /* No error */
    motor->last_error = MOTEUS_OK;
    return MOTEUS_OK;
}

/**
 * @brief Handle timeout error (internal helper)
 *
 * Sets error state, invokes callback, and attempts auto-recovery if enabled.
 * @return true if recovered successfully, false otherwise
 */
static bool handle_timeout(moteus_motor_t* motor)
{
    motor->last_error = MOTEUS_ERR_TIMEOUT;
    motor->timeout_count++;

    if (motor->error_cb != NULL) {
        motor->error_cb(motor, MOTEUS_ERR_TIMEOUT, motor->error_user_data);
    }

    /* Attempt auto-recovery if enabled */
    if (motor->auto_recover) {
        const moteus_result_t* result = moteus_recover(motor);
        if (result != NULL &&
            result->mode != MOTEUS_MODE_FAULT &&
            result->mode != MOTEUS_MODE_TIMEOUT) {
            motor->last_error = MOTEUS_ERR_RECOVERED;
            return true;
        }
    }

    return false;
}

/**
 * @brief Handle error detected in result (internal helper)
 *
 * Attempts auto-recovery if enabled.
 * @return true if recovered successfully, false otherwise
 */
static bool handle_error(moteus_motor_t* motor)
{
    /* Attempt auto-recovery if enabled */
    if (motor->auto_recover) {
        const moteus_result_t* result = moteus_recover(motor);
        if (result != NULL &&
            result->mode != MOTEUS_MODE_FAULT &&
            result->mode != MOTEUS_MODE_TIMEOUT) {
            motor->last_error = MOTEUS_ERR_RECOVERED;
            return true;
        }
    }

    return false;
}

/* ----------------------------------------------------------------------------
 * Internal send helpers
 * ---------------------------------------------------------------------------- */

static HAL_StatusTypeDef send_stop(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_stop_frame(&frame, motor->id, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_brake(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_brake_frame(&frame, motor->id, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_query(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_query_frame(&frame, motor->id, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_position(moteus_motor_t* motor,
                                        const moteus_position_cmd_t* cmd)
{
    moteus_can_frame_t frame;
    int len = moteus_build_position_frame(&frame, motor->id, cmd,
                                          &motor->cmd_res, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_torque(moteus_motor_t* motor, float torque)
{
    moteus_can_frame_t frame;
    int len = moteus_build_torque_frame(&frame, motor->id, torque, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_stay_within(moteus_motor_t* motor,
                                           const moteus_stay_within_cmd_t* cmd)
{
    moteus_can_frame_t frame;
    moteus_stay_within_resolution_t res = MOTEUS_STAY_WITHIN_RESOLUTION_DEFAULT;
    int len = moteus_build_stay_within_frame(&frame, motor->id, cmd, &res, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_vfoc(moteus_motor_t* motor,
                                    const moteus_vfoc_cmd_t* cmd)
{
    moteus_can_frame_t frame;
    moteus_vfoc_resolution_t res = MOTEUS_VFOC_RESOLUTION_DEFAULT;
    int len = moteus_build_vfoc_frame(&frame, motor->id, cmd, &res, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_rezero(moteus_motor_t* motor, float position)
{
    moteus_can_frame_t frame;
    int len = moteus_build_rezero_frame(&frame, motor->id, position, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_output_exact(moteus_motor_t* motor, float position)
{
    moteus_can_frame_t frame;
    int len = moteus_build_set_output_exact_frame(&frame, motor->id, position, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_require_reindex(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_require_reindex_frame(&frame, motor->id, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_recapture(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_recapture_frame(&frame, motor->id, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_gpio_write(moteus_motor_t* motor,
                                          const moteus_gpio_cmd_t* cmd)
{
    moteus_can_frame_t frame;
    int len = moteus_build_gpio_write_frame(&frame, motor->id, cmd, &motor->query_res);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_gpio_read(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_gpio_read_frame(&frame, motor->id);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_analog_read(moteus_motor_t* motor)
{
    moteus_can_frame_t frame;
    int len = moteus_build_analog_read_frame(&frame, motor->id, true, true);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_diagnostic_write(moteus_motor_t* motor,
                                                const moteus_diagnostic_cmd_t* cmd)
{
    moteus_can_frame_t frame;
    int len = moteus_build_diagnostic_write_frame(&frame, motor->id, cmd->channel,
                                                  cmd->data, cmd->len);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

static HAL_StatusTypeDef send_diagnostic_read(moteus_motor_t* motor, uint8_t channel)
{
    moteus_can_frame_t frame;
    int len = moteus_build_diagnostic_read_frame(&frame, motor->id, channel);
    if (len < 0) return HAL_ERROR;

    motor->response_received = false;
    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, &frame);
}

/* ----------------------------------------------------------------------------
 * Blocking commands (Set*) - return result pointer or NULL
 * ---------------------------------------------------------------------------- */

const moteus_result_t* moteus_set_stop(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    if (send_stop(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        /* Don't auto-recover for stop - it IS the recovery mechanism */
        motor->last_error = MOTEUS_ERR_TIMEOUT;
        motor->timeout_count++;
        if (motor->error_cb != NULL) {
            motor->error_cb(motor, MOTEUS_ERR_TIMEOUT, motor->error_user_data);
        }
        return NULL;
    }

    /* Check for errors but don't auto-recover (stop clears faults) */
    check_result_for_errors(motor);
    return &motor->result;
}

const moteus_result_t* moteus_set_brake(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    if (send_brake(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_query(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    if (send_query(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    /* Query just reads state - report but don't auto-recover */
    check_result_for_errors(motor);
    return &motor->result;
}

const moteus_result_t* moteus_set_position(moteus_motor_t* motor,
                                            const moteus_position_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return NULL;

    if (send_position(motor, cmd) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_torque(moteus_motor_t* motor, float torque)
{
    if (motor == NULL) return NULL;

    if (send_torque(motor, torque) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_stay_within(moteus_motor_t* motor,
                                               const moteus_stay_within_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return NULL;

    if (send_stay_within(motor, cmd) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_vfoc(moteus_motor_t* motor,
                                        const moteus_vfoc_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return NULL;

    if (send_vfoc(motor, cmd) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_rezero(moteus_motor_t* motor, float position)
{
    if (motor == NULL) return NULL;

    if (send_rezero(motor, position) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_output_exact(moteus_motor_t* motor, float position)
{
    if (motor == NULL) return NULL;

    if (send_output_exact(motor, position) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_require_reindex(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    if (send_require_reindex(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_recapture(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    if (send_recapture(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_set_gpio(moteus_motor_t* motor,
                                        const moteus_gpio_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return NULL;

    if (send_gpio_write(motor, cmd) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    return &motor->result;
}

const moteus_result_t* moteus_read_gpio(moteus_motor_t* motor,
                                         moteus_gpio_result_t* result)
{
    if (motor == NULL) return NULL;

    if (send_gpio_read(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    /* Parse GPIO values from stored frame data */
    if (result != NULL) {
        moteus_parse_gpio_response(motor->rx_frame_data, motor->rx_frame_len, result);
    }
    return &motor->result;
}

const moteus_result_t* moteus_read_analog(moteus_motor_t* motor,
                                          moteus_analog_result_t* result)
{
    if (motor == NULL) return NULL;

    if (send_analog_read(motor) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    /* Parse analog values from stored frame data */
    if (result != NULL) {
        moteus_parse_analog_response(motor->rx_frame_data, motor->rx_frame_len, result);
    }
    return &motor->result;
}

const moteus_result_t* moteus_diagnostic_write(moteus_motor_t* motor,
                                                const moteus_diagnostic_cmd_t* cmd,
                                                moteus_diagnostic_result_t* response)
{
    if (motor == NULL || cmd == NULL) return NULL;

    if (send_diagnostic_write(motor, cmd) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    /* Parse diagnostic response from raw frame data */
    if (response != NULL) {
        if (moteus_parse_diagnostic_response(motor->rx_frame_data,
                                              motor->rx_frame_len,
                                              response) != MOTEUS_OK) {
            response->channel = cmd->channel;
            response->len = 0;
        }
    }
    return &motor->result;
}

const moteus_result_t* moteus_diagnostic_read(moteus_motor_t* motor,
                                               uint8_t channel,
                                               moteus_diagnostic_result_t* response)
{
    if (motor == NULL || response == NULL) return NULL;

    if (send_diagnostic_read(motor, channel) != HAL_OK) {
        motor->last_error = MOTEUS_ERR_TX_FAILED;
        return NULL;
    }

    if (!wait_for_response(motor)) {
        handle_timeout(motor);
        return NULL;
    }

    if (check_result_for_errors(motor) != MOTEUS_OK) {
        handle_error(motor);
    }

    /* Parse diagnostic response from raw frame data */
    if (moteus_parse_diagnostic_response(motor->rx_frame_data,
                                          motor->rx_frame_len,
                                          response) != MOTEUS_OK) {
        response->channel = channel;
        response->len = 0;
    }
    return &motor->result;
}

const moteus_result_t* moteus_diagnostic_command(moteus_motor_t* motor,
                                                  const char* command,
                                                  char* response,
                                                  size_t response_len)
{
    if (motor == NULL || command == NULL) return NULL;

    const uint8_t channel = MOTEUS_DIAG_CHANNEL_CONFIG;

    /* Initialize response buffer */
    size_t resp_pos = 0;
    if (response != NULL && response_len > 0) {
        response[0] = '\0';
    }

    /* Aggressively flush any stale data - keep reading until empty 3x in a row */
    moteus_diagnostic_result_t flush_resp;
    int empty_count = 0;
    uint32_t flush_start = HAL_GetTick();
    while (empty_count < 3 && (HAL_GetTick() - flush_start) < 500) {
        if (moteus_diagnostic_read(motor, channel, &flush_resp) == NULL) {
            empty_count++;
        } else if (flush_resp.len == 0) {
            empty_count++;
        } else {
            empty_count = 0;  /* Reset - got data, keep flushing */
        }
        HAL_Delay(2);
    }

    /* Build and send diagnostic command */
    moteus_diagnostic_cmd_t cmd;
    cmd.channel = channel;
    size_t cmd_len = strlen(command);
    if (cmd_len > sizeof(cmd.data) - 1) {
        cmd_len = sizeof(cmd.data) - 1;
    }
    memcpy(cmd.data, command, cmd_len);
    cmd.data[cmd_len] = '\n';  /* Commands end with newline */
    cmd.len = cmd_len + 1;

    moteus_diagnostic_result_t diag_response;
    const moteus_result_t* result = moteus_diagnostic_write(motor, &cmd, &diag_response);
    if (result == NULL) return NULL;

    /* Copy first chunk from write response */
    if (response != NULL && diag_response.len > 0) {
        size_t copy_len = diag_response.len;
        if (copy_len >= response_len - resp_pos) {
            copy_len = response_len - resp_pos - 1;
        }
        memcpy(response + resp_pos, diag_response.data, copy_len);
        resp_pos += copy_len;
        response[resp_pos] = '\0';
    }

    /* For simple "conf get" commands, response is typically:
     * "<value>\r\nOK\r\n"
     * This usually fits in one or two chunks. Poll until complete. */
    uint32_t start_time = HAL_GetTick();
    const uint32_t timeout_ms = 500;

    while ((HAL_GetTick() - start_time) < timeout_ms) {
        /* Check if response is complete */
        if (response != NULL) {
            if (strstr(response, "OK\r\n") != NULL ||
                strstr(response, "OK\n") != NULL ||
                strstr(response, "ERR ") != NULL) {
                break;
            }
        }

        /* Poll for more data */
        HAL_Delay(5);
        result = moteus_diagnostic_read(motor, channel, &diag_response);
        if (result == NULL) continue;

        /* No more data available */
        if (diag_response.len == 0) {
            continue;
        }

        /* Append chunk */
        if (response != NULL && resp_pos < response_len - 1) {
            size_t copy_len = diag_response.len;
            if (copy_len >= response_len - resp_pos) {
                copy_len = response_len - resp_pos - 1;
            }
            memcpy(response + resp_pos, diag_response.data, copy_len);
            resp_pos += copy_len;
            response[resp_pos] = '\0';
        }
    }

    return &motor->result;
}

/* ----------------------------------------------------------------------------
 * Non-blocking commands (Begin*)
 * ---------------------------------------------------------------------------- */

HAL_StatusTypeDef moteus_begin_stop(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_stop(motor);
}

HAL_StatusTypeDef moteus_begin_brake(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_brake(motor);
}

HAL_StatusTypeDef moteus_begin_query(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_query(motor);
}

HAL_StatusTypeDef moteus_begin_position(moteus_motor_t* motor,
                                         const moteus_position_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return HAL_ERROR;
    return send_position(motor, cmd);
}

HAL_StatusTypeDef moteus_begin_torque(moteus_motor_t* motor, float torque)
{
    if (motor == NULL) return HAL_ERROR;
    return send_torque(motor, torque);
}

HAL_StatusTypeDef moteus_begin_stay_within(moteus_motor_t* motor,
                                            const moteus_stay_within_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return HAL_ERROR;
    return send_stay_within(motor, cmd);
}

HAL_StatusTypeDef moteus_begin_vfoc(moteus_motor_t* motor,
                                     const moteus_vfoc_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return HAL_ERROR;
    return send_vfoc(motor, cmd);
}

HAL_StatusTypeDef moteus_begin_rezero(moteus_motor_t* motor, float position)
{
    if (motor == NULL) return HAL_ERROR;
    return send_rezero(motor, position);
}

HAL_StatusTypeDef moteus_begin_output_exact(moteus_motor_t* motor, float position)
{
    if (motor == NULL) return HAL_ERROR;
    return send_output_exact(motor, position);
}

HAL_StatusTypeDef moteus_begin_require_reindex(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_require_reindex(motor);
}

HAL_StatusTypeDef moteus_begin_recapture(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_recapture(motor);
}

HAL_StatusTypeDef moteus_begin_gpio(moteus_motor_t* motor,
                                     const moteus_gpio_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return HAL_ERROR;
    return send_gpio_write(motor, cmd);
}

HAL_StatusTypeDef moteus_begin_gpio_read(moteus_motor_t* motor)
{
    if (motor == NULL) return HAL_ERROR;
    return send_gpio_read(motor);
}

HAL_StatusTypeDef moteus_begin_diagnostic_write(moteus_motor_t* motor,
                                                 const moteus_diagnostic_cmd_t* cmd)
{
    if (motor == NULL || cmd == NULL) return HAL_ERROR;
    return send_diagnostic_write(motor, cmd);
}

HAL_StatusTypeDef moteus_begin_diagnostic_read(moteus_motor_t* motor,
                                                uint8_t channel)
{
    if (motor == NULL) return HAL_ERROR;
    return send_diagnostic_read(motor, channel);
}

HAL_StatusTypeDef moteus_send_raw(moteus_motor_t* motor,
                                   const moteus_can_frame_t* frame)
{
    if (motor == NULL || frame == NULL) return HAL_ERROR;

    motor->tx_count++;
    return moteus_can_transmit(motor->hfdcan, frame);
}

/* ============================================================================
 * Response Processing
 * ============================================================================ */

void moteus_process_rx(const FDCAN_RxHeaderTypeDef* header,
                        const uint8_t* data,
                        uint32_t data_len)
{
    if (header == NULL || data == NULL || data_len == 0) {
        return;
    }

    /* Note: We accept both standard and extended ID frames.
     * Moteus responses with small IDs (e.g., 0x100) may be received
     * as standard ID frames by the hardware even though Moteus uses
     * extended IDs. The ID parsing works correctly either way. */

    /* Extract source ID (the motor that sent this response) */
    uint8_t source_id = moteus_get_source_id(header->Identifier);

    /* Look up motor instance */
    moteus_motor_t* motor = moteus_get_motor_by_id(source_id);
    if (motor == NULL) {
        return;  /* Unknown motor, ignore */
    }

    /* Store raw frame data for later parsing (GPIO, analog, diagnostic, etc.) */
    uint8_t copy_len = (data_len < 64) ? data_len : 64;
    memcpy(motor->rx_frame_data, data, copy_len);
    motor->rx_frame_len = copy_len;

    /* Mark response received (even if parse fails - raw data is available) */
    motor->response_received = true;
    motor->rx_count++;

    /* Try to parse as standard query response */
    moteus_result_t result;
    int parse_result = moteus_parse_response(data, data_len, &result);

    if (parse_result == MOTEUS_OK) {
        /* Update last known state for standard responses */
        motor->result = result;

        /* Invoke callback (if registered) for standard responses only */
        if (motor->response_cb != NULL) {
            motor->response_cb(motor, &result, motor->response_user_data);
        }
    }
    /* Non-standard responses (diagnostic, etc.) are handled via raw frame data */
}

bool moteus_poll(moteus_motor_t* motor)
{
    if (motor == NULL) return false;

    if (motor->response_received) {
        motor->response_received = false;
        return true;
    }
    return false;
}

/* ============================================================================
 * Motor Registry
 * ============================================================================ */

moteus_motor_t* moteus_get_motor_by_id(uint8_t id)
{
    if (id == 0 || id > 127) {
        return NULL;
    }
    return motor_registry[id];
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

const char* moteus_mode_str(uint8_t mode)
{
    switch (mode) {
        case MOTEUS_MODE_STOPPED:             return "Stopped";
        case MOTEUS_MODE_FAULT:               return "Fault";
        case MOTEUS_MODE_ENABLING:            return "Enabling";
        case MOTEUS_MODE_CALIBRATING:         return "Calibrating";
        case MOTEUS_MODE_CALIBRATION_COMPLETE: return "Calibration Complete";
        case MOTEUS_MODE_PWM:                 return "PWM";
        case MOTEUS_MODE_VOLTAGE:             return "Voltage";
        case MOTEUS_MODE_VOLTAGE_FOC:         return "Voltage FOC";
        case MOTEUS_MODE_VOLTAGE_DQ:          return "Voltage DQ";
        case MOTEUS_MODE_CURRENT:             return "Current";
        case MOTEUS_MODE_POSITION:            return "Position";
        case MOTEUS_MODE_TIMEOUT:             return "Timeout";
        case MOTEUS_MODE_ZERO_VELOCITY:       return "Zero Velocity";
        case MOTEUS_MODE_STAY_WITHIN:         return "Stay Within";
        case MOTEUS_MODE_MEASURE_IND:         return "Measure Inductance";
        case MOTEUS_MODE_BRAKE:               return "Brake";
        default:                              return "Unknown";
    }
}

const char* moteus_fault_str(uint8_t fault)
{
    switch (fault) {
        case MOTEUS_FAULT_NONE:                return "None";
        case MOTEUS_FAULT_DMA_ERROR:           return "DMA Error";
        case MOTEUS_FAULT_ENCODER_NOT_FOUND:   return "Encoder Not Found";
        case MOTEUS_FAULT_MOTOR_NOT_CONFIGURED: return "Motor Not Configured";
        case MOTEUS_FAULT_PWM_CYCLE_OVERRUN:   return "PWM Cycle Overrun";
        case MOTEUS_FAULT_OVER_VOLTAGE:        return "Over Voltage";
        case MOTEUS_FAULT_ENCODER_FAULT:       return "Encoder Fault";
        case MOTEUS_FAULT_MOTOR_DRIVER_FAULT:  return "Motor Driver Fault";
        case MOTEUS_FAULT_OVER_TEMP:           return "Over Temperature";
        case MOTEUS_FAULT_START_OUT_POSITION:  return "Start Outside Position";
        case MOTEUS_FAULT_UNDER_VOLTAGE:       return "Under Voltage";
        case MOTEUS_FAULT_CONFIG_CHANGED:      return "Config Changed";
        case MOTEUS_FAULT_MOTOR_DRIVER_ENABLE: return "Motor Driver Enable";
        case MOTEUS_FAULT_MOTOR_DRIVER_FAULT2: return "Motor Driver Fault 2";
        default:                               return "Unknown";
    }
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

const char* moteus_error_str(int error)
{
    switch (error) {
        case MOTEUS_OK:                   return "OK";
        case MOTEUS_ERR_INVALID_PARAM:    return "Invalid parameter";
        case MOTEUS_ERR_BUFFER_FULL:      return "Buffer full";
        case MOTEUS_ERR_TX_FAILED:        return "Transmit failed";
        case MOTEUS_ERR_TIMEOUT:          return "Timeout";
        case MOTEUS_ERR_INVALID_FRAME:    return "Invalid frame";
        case MOTEUS_ERR_NO_MOTOR:         return "Motor not found";
        case MOTEUS_ERR_NOT_INITIALIZED:  return "Not initialized";
        case MOTEUS_ERR_DIAGNOSTIC:       return "Diagnostic error";
        case MOTEUS_ERR_MOTOR_FAULT:      return "Motor fault";
        case MOTEUS_ERR_MOTOR_TIMEOUT:    return "Motor timeout";
        case MOTEUS_ERR_CAN_BUS:          return "CAN bus error";
        case MOTEUS_ERR_RECOVERED:        return "Recovered from error";
        default:                          return "Unknown error";
    }
}

int moteus_get_last_error(moteus_motor_t* motor)
{
    if (motor == NULL) return MOTEUS_ERR_INVALID_PARAM;
    return motor->last_error;
}

uint8_t moteus_get_last_fault(moteus_motor_t* motor)
{
    if (motor == NULL) return MOTEUS_FAULT_NONE;
    return motor->last_fault;
}

void moteus_clear_error(moteus_motor_t* motor)
{
    if (motor == NULL) return;
    motor->last_error = MOTEUS_OK;
    motor->last_fault = MOTEUS_FAULT_NONE;
    motor->response_received = false;
}

void moteus_set_auto_recover(moteus_motor_t* motor, bool enable)
{
    if (motor == NULL) return;
    motor->auto_recover = enable;
}

bool moteus_get_auto_recover(moteus_motor_t* motor)
{
    if (motor == NULL) return false;
    return motor->auto_recover;
}

const moteus_result_t* moteus_recover(moteus_motor_t* motor)
{
    if (motor == NULL) return NULL;

    /* Send stop command to clear faults */
    const moteus_result_t* result = moteus_set_stop(motor);

    if (result != NULL) {
        /* Check if recovery was successful */
        if (result->mode != MOTEUS_MODE_FAULT && result->mode != MOTEUS_MODE_TIMEOUT) {
            motor->recovery_count++;
            motor->last_error = MOTEUS_OK;
            motor->last_fault = MOTEUS_FAULT_NONE;
        }
    }

    return result;
}

int moteus_check_status(moteus_motor_t* motor)
{
    if (motor == NULL) return MOTEUS_ERR_INVALID_PARAM;

    /* Query motor state */
    const moteus_result_t* result = moteus_set_query(motor);

    if (result == NULL) {
        motor->last_error = MOTEUS_ERR_TIMEOUT;
        motor->timeout_count++;
        return MOTEUS_ERR_TIMEOUT;
    }

    /* Check for fault mode */
    if (result->mode == MOTEUS_MODE_FAULT) {
        motor->last_error = MOTEUS_ERR_MOTOR_FAULT;
        motor->last_fault = result->fault;
        motor->fault_count++;
        return MOTEUS_ERR_MOTOR_FAULT;
    }

    /* Check for timeout mode */
    if (result->mode == MOTEUS_MODE_TIMEOUT) {
        motor->last_error = MOTEUS_ERR_MOTOR_TIMEOUT;
        motor->timeout_count++;
        return MOTEUS_ERR_MOTOR_TIMEOUT;
    }

    motor->last_error = MOTEUS_OK;
    return MOTEUS_OK;
}

bool moteus_is_error_state(moteus_motor_t* motor)
{
    if (motor == NULL) return true;
    return (motor->result.mode == MOTEUS_MODE_FAULT ||
            motor->result.mode == MOTEUS_MODE_TIMEOUT);
}
