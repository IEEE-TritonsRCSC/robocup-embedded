/**
 * @file moteus.h
 * @brief Moteus motor controller library for STM32
 *
 * High-level API for controlling Moteus brushless motor controllers
 * via CAN-FD on STM32 microcontrollers using HAL.
 *
 * @example
 * @code
 * // Initialize CAN and motor (bit timing configured in CubeMX)
 * moteus_can_init(&hfdcan1);
 * moteus_motor_t* motor = moteus_init(&hfdcan1, 1);
 *
 * // Set up callback
 * moteus_set_response_callback(motor, on_motor_response, NULL);
 *
 * // Send position command
 * moteus_position_cmd_t cmd = MOTEUS_POSITION_CMD_DEFAULT;
 * cmd.position = 0.5f;  // Half revolution
 * moteus_set_position(motor, &cmd);
 *
 * // In CAN RX callback or polling loop
 * moteus_process_rx(&rx_header, rx_data, rx_len);
 * @endcode
 */

#ifndef MOTEUS_H
#define MOTEUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* STM32 HAL includes - user must include appropriate HAL header before this */
#ifdef STM32G4xx_HAL_FDCAN_H
    /* Already included */
#else
    /* Try to include it - will fail if HAL not configured */
    #if defined(STM32G431xx) || defined(STM32G441xx) || defined(STM32G471xx) || \
        defined(STM32G473xx) || defined(STM32G474xx) || defined(STM32G483xx) || \
        defined(STM32G484xx) || defined(STM32G491xx) || defined(STM32G4A1xx)
        #include "stm32g4xx_hal.h"
    #endif
#endif

#include "moteus_types.h"
#include "moteus_registers.h"
#include "moteus_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

/** Maximum number of motors that can be registered */
#ifndef MOTEUS_MAX_MOTORS
#define MOTEUS_MAX_MOTORS 2
#endif

/** Source ID used in CAN frames (arbitrary, usually 0) */
#ifndef MOTEUS_SOURCE_ID
#define MOTEUS_SOURCE_ID 0
#endif

/** Default timeout for blocking commands (ms) */
#ifndef MOTEUS_DEFAULT_TIMEOUT_MS
#define MOTEUS_DEFAULT_TIMEOUT_MS 100
#endif

/** Enable BRS (Bit Rate Switching) for 5 Mbps data phase
 *  When enabled, data phase runs at 5 Mbps, arbitration at 1 Mbps.
 *  Requires specific FDCAN timing configuration - see README.
 */
#ifndef MOTEUS_ENABLE_BRS
#define MOTEUS_ENABLE_BRS 1
#endif

/* ============================================================================
 * Motor Instance
 * ============================================================================ */

/**
 * @brief Motor instance structure
 *
 * Represents a single Moteus motor controller.
 * Create with moteus_init(), don't allocate directly.
 */
struct moteus_motor {
    FDCAN_HandleTypeDef* hfdcan;         /**< HAL FDCAN handle */
    uint8_t id;                          /**< Motor CAN ID (1-127) */
    bool registered;                     /**< True if registered in global list */

    /* Configuration */
    moteus_query_resolution_t query_res;     /**< Query resolution config */
    moteus_position_resolution_t cmd_res;    /**< Command resolution config */

    /* Callbacks */
    moteus_response_cb_t response_cb;    /**< Response callback */
    void* response_user_data;            /**< Response callback user data */
    moteus_error_cb_t error_cb;          /**< Error callback */
    void* error_user_data;               /**< Error callback user data */

    /* State */
    moteus_result_t result;              /**< Latest motor state (updated on each response) */
    volatile bool response_received;     /**< Set when response arrives, cleared by poll/wait */
    uint8_t rx_frame_data[64];           /**< Raw frame data from last response */
    uint8_t rx_frame_len;                /**< Length of last response frame */

    /* Configuration */
    uint32_t timeout_ms;                 /**< Timeout for blocking commands */

    /* Statistics */
    uint32_t rx_count;                   /**< Number of responses received */
    uint32_t tx_count;                   /**< Number of commands sent */
    uint32_t error_count;                /**< Number of errors */
    uint32_t timeout_count;              /**< Number of timeouts */
    uint32_t fault_count;                /**< Number of motor faults */
    uint32_t recovery_count;             /**< Number of successful recoveries */

    /* Error tracking */
    int last_error;                      /**< Last error code (MOTEUS_ERR_*) */
    uint8_t last_fault;                  /**< Last motor fault code (MOTEUS_FAULT_*) */
    bool auto_recover;                   /**< Auto-recover on timeout/fault */
};

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * @brief Initialize CAN peripheral for Moteus communication
 *
 * Sets up RX filters, starts the FDCAN peripheral, and enables RX interrupts.
 * Must be called before moteus_init().
 *
 * @param hfdcan HAL FDCAN handle (must be initialized with CubeMX/HAL)
 * @return HAL_OK on success
 *
 * @note Bit timing must be configured in CubeMX. For 1 Mbps with 170 MHz clock:
 *       Prescaler=1, TimeSeg1=135, TimeSeg2=34, SyncJumpWidth=34
 */
HAL_StatusTypeDef moteus_can_init(FDCAN_HandleTypeDef* hfdcan);

/**
 * @brief Transmit a CAN frame
 *
 * Low-level transmit function. Normally use moteus_set_* or moteus_begin_* instead.
 *
 * @param hfdcan HAL FDCAN handle
 * @param frame Frame to transmit
 * @return HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_can_transmit(FDCAN_HandleTypeDef* hfdcan,
                                       const moteus_can_frame_t* frame);

/**
 * @brief Create a motor instance
 *
 * Allocates and initializes a motor instance with default configuration.
 * The motor is automatically registered for RX routing.
 *
 * @param hfdcan HAL FDCAN handle
 * @param motor_id Motor CAN ID (1-127)
 * @return Pointer to motor instance, or NULL on error
 *
 * @note Motor instances are statically allocated from a pool.
 *       Maximum number is MOTEUS_MAX_MOTORS.
 */
moteus_motor_t* moteus_init(FDCAN_HandleTypeDef* hfdcan, uint8_t motor_id);

/**
 * @brief Deinitialize a motor instance
 *
 * Unregisters the motor and frees resources.
 *
 * @param motor Motor instance to deinitialize
 */
void moteus_deinit(moteus_motor_t* motor);

/* ============================================================================
 * Configuration
 * ============================================================================ */

/**
 * @brief Set query resolution for motor responses
 *
 * @param motor Motor instance
 * @param res Query resolution configuration
 */
void moteus_set_query_resolution(moteus_motor_t* motor,
                                  const moteus_query_resolution_t* res);

/**
 * @brief Set command resolution for position commands
 *
 * @param motor Motor instance
 * @param res Command resolution configuration
 */
void moteus_set_command_resolution(moteus_motor_t* motor,
                                    const moteus_position_resolution_t* res);

/**
 * @brief Set response callback
 *
 * @param motor Motor instance
 * @param callback Callback function (NULL to disable)
 * @param user_data User data passed to callback
 */
void moteus_set_response_callback(moteus_motor_t* motor,
                                   moteus_response_cb_t callback,
                                   void* user_data);

/**
 * @brief Set error callback
 *
 * @param motor Motor instance
 * @param callback Callback function (NULL to disable)
 * @param user_data User data passed to callback
 */
void moteus_set_error_callback(moteus_motor_t* motor,
                                moteus_error_cb_t callback,
                                void* user_data);

/* ============================================================================
 * Commands - Blocking (Set*)
 *
 * Send command and wait for response. Returns pointer to result on success,
 * NULL on timeout or error. Result is also available via motor->result.
 * ============================================================================ */

/**
 * @brief Stop motor and wait for response
 *
 * Disables PWM output. Motor will coast to a stop.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_stop(moteus_motor_t* motor);

/**
 * @brief Brake motor and wait for response
 *
 * Puts motor in brake mode which resists movement but allows backdrive.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_brake(moteus_motor_t* motor);

/**
 * @brief Query motor state and wait for response
 *
 * Queries current motor state without changing mode.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_query(moteus_motor_t* motor);

/**
 * @brief Send position command and wait for response
 *
 * Controls motor position, velocity, and/or torque.
 *
 * @param motor Motor instance
 * @param cmd Position command parameters
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_position(moteus_motor_t* motor,
                                            const moteus_position_cmd_t* cmd);

/**
 * @brief Send torque command and wait for response
 *
 * Pure torque control mode (position mode with kp=0, kd=0).
 *
 * @param motor Motor instance
 * @param torque Target torque in Nm
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_torque(moteus_motor_t* motor, float torque);

/**
 * @brief Send stay-within command and wait for response
 *
 * Keeps motor within bounded position range with configurable gains.
 *
 * @param motor Motor instance
 * @param cmd Stay-within command parameters
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_stay_within(moteus_motor_t* motor,
                                               const moteus_stay_within_cmd_t* cmd);

/**
 * @brief Send VFOC (Voltage FOC) command and wait for response
 *
 * Direct voltage control with specified electrical angle.
 *
 * @param motor Motor instance
 * @param cmd VFOC command parameters
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_vfoc(moteus_motor_t* motor,
                                        const moteus_vfoc_cmd_t* cmd);

/**
 * @brief Rezero motor position and wait for response
 *
 * Sets output position to nearest encoder index. Use for homing.
 *
 * @param motor Motor instance
 * @param position Position value to set (use NaN for nearest index)
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_rezero(moteus_motor_t* motor, float position);

/**
 * @brief Set exact output position and wait for response
 *
 * Sets output position to exact value without seeking index.
 *
 * @param motor Motor instance
 * @param position Position value to set
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_output_exact(moteus_motor_t* motor, float position);

/**
 * @brief Require encoder reindex and wait for response
 *
 * Forces encoder to seek index before accepting commands.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_require_reindex(moteus_motor_t* motor);

/**
 * @brief Recapture position and wait for response
 *
 * Re-synchronizes position feedback from encoder.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_recapture(moteus_motor_t* motor);

/**
 * @brief Write GPIO outputs and wait for response
 *
 * Sets digital output pins on AUX1/AUX2 ports.
 *
 * @param motor Motor instance
 * @param cmd GPIO command (aux1/aux2 output bits)
 * @return Pointer to result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_set_gpio(moteus_motor_t* motor,
                                        const moteus_gpio_cmd_t* cmd);

/**
 * @brief Read GPIO inputs and wait for response
 *
 * Reads digital input pins from AUX1/AUX2 ports.
 *
 * @param motor Motor instance
 * @param result Output: GPIO input values
 * @return Pointer to motor result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_read_gpio(moteus_motor_t* motor,
                                         moteus_gpio_result_t* result);

/**
 * @brief Read analog inputs and wait for response
 *
 * Reads analog input pins from AUX1/AUX2 ports.
 * Values are returned as floats in the 0.0-1.0 range.
 *
 * @param motor Motor instance
 * @param result Output: Analog input values (aux1[5], aux2[5])
 * @return Pointer to motor result on success, NULL on timeout/error
 *
 * @note Not all Moteus boards support analog inputs on all pins.
 *       Check your board's documentation for pin capabilities.
 */
const moteus_result_t* moteus_read_analog(moteus_motor_t* motor,
                                          moteus_analog_result_t* result);

/**
 * @brief Write diagnostic data and wait for response
 *
 * Sends configuration data to the controller's diagnostic stream.
 * Used for setting parameters like gains.
 *
 * @param motor Motor instance
 * @param cmd Diagnostic command (channel + data)
 * @param response Output: Diagnostic response (can be NULL)
 * @return Pointer to motor result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_diagnostic_write(moteus_motor_t* motor,
                                                const moteus_diagnostic_cmd_t* cmd,
                                                moteus_diagnostic_result_t* response);

/**
 * @brief Read diagnostic data and wait for response
 *
 * Reads configuration data from the controller's diagnostic stream.
 *
 * @param motor Motor instance
 * @param channel Diagnostic channel (1=config, 2=telemetry)
 * @param response Output: Diagnostic response
 * @return Pointer to motor result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_diagnostic_read(moteus_motor_t* motor,
                                               uint8_t channel,
                                               moteus_diagnostic_result_t* response);

/**
 * @brief Send diagnostic command string and wait for response
 *
 * Convenience function for sending text commands to the diagnostic stream.
 * Example: "conf set servo.pid_position.kp 10.0"
 *
 * @param motor Motor instance
 * @param command Null-terminated command string
 * @param response Output buffer for response string
 * @param response_len Maximum response buffer length
 * @return Pointer to motor result on success, NULL on timeout/error
 */
const moteus_result_t* moteus_diagnostic_command(moteus_motor_t* motor,
                                                  const char* command,
                                                  char* response,
                                                  size_t response_len);

/* ============================================================================
 * Commands - Non-blocking (Begin*)
 *
 * Send command and return immediately. Use moteus_poll() to check for response.
 * Returns HAL_OK on successful transmit, HAL_ERROR on failure.
 * ============================================================================ */

/**
 * @brief Send stop command (non-blocking)
 *
 * Sends stop command and returns immediately. Use moteus_poll() to check for response.
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_stop(moteus_motor_t* motor);

/**
 * @brief Send brake command (non-blocking)
 *
 * Sends brake command and returns immediately. Use moteus_poll() to check for response.
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_brake(moteus_motor_t* motor);

/**
 * @brief Send query (non-blocking)
 *
 * Sends query and returns immediately. Use moteus_poll() to check for response.
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_query(moteus_motor_t* motor);

/**
 * @brief Send position command (non-blocking)
 *
 * Sends position command and returns immediately. Use moteus_poll() to check for response.
 *
 * @param motor Motor instance
 * @param cmd Position command parameters
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_position(moteus_motor_t* motor,
                                         const moteus_position_cmd_t* cmd);

/**
 * @brief Send torque command (non-blocking)
 *
 * Sends torque command and returns immediately. Use moteus_poll() to check for response.
 *
 * @param motor Motor instance
 * @param torque Target torque in Nm
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_torque(moteus_motor_t* motor, float torque);

/**
 * @brief Send stay-within command (non-blocking)
 *
 * @param motor Motor instance
 * @param cmd Stay-within command parameters
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_stay_within(moteus_motor_t* motor,
                                            const moteus_stay_within_cmd_t* cmd);

/**
 * @brief Send VFOC command (non-blocking)
 *
 * @param motor Motor instance
 * @param cmd VFOC command parameters
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_vfoc(moteus_motor_t* motor,
                                     const moteus_vfoc_cmd_t* cmd);

/**
 * @brief Send rezero command (non-blocking)
 *
 * @param motor Motor instance
 * @param position Position value to set (use NaN for nearest index)
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_rezero(moteus_motor_t* motor, float position);

/**
 * @brief Send set output exact command (non-blocking)
 *
 * @param motor Motor instance
 * @param position Position value to set
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_output_exact(moteus_motor_t* motor, float position);

/**
 * @brief Send require reindex command (non-blocking)
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_require_reindex(moteus_motor_t* motor);

/**
 * @brief Send recapture command (non-blocking)
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_recapture(moteus_motor_t* motor);

/**
 * @brief Send GPIO write command (non-blocking)
 *
 * @param motor Motor instance
 * @param cmd GPIO command (aux1/aux2 output bits)
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_gpio(moteus_motor_t* motor,
                                     const moteus_gpio_cmd_t* cmd);

/**
 * @brief Send GPIO read command (non-blocking)
 *
 * @param motor Motor instance
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_gpio_read(moteus_motor_t* motor);

/**
 * @brief Send diagnostic write command (non-blocking)
 *
 * @param motor Motor instance
 * @param cmd Diagnostic command (channel + data)
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_diagnostic_write(moteus_motor_t* motor,
                                                 const moteus_diagnostic_cmd_t* cmd);

/**
 * @brief Send diagnostic read/poll command (non-blocking)
 *
 * @param motor Motor instance
 * @param channel Diagnostic channel (1=config, 2=telemetry)
 * @return HAL_OK on successful transmit, HAL_ERROR on failure
 */
HAL_StatusTypeDef moteus_begin_diagnostic_read(moteus_motor_t* motor,
                                                uint8_t channel);

/**
 * @brief Send raw CAN frame
 *
 * For advanced use - sends a pre-built CAN frame.
 *
 * @param motor Motor instance
 * @param frame CAN frame to send
 * @return HAL_OK on success
 */
HAL_StatusTypeDef moteus_send_raw(moteus_motor_t* motor,
                                   const moteus_can_frame_t* frame);

/* ============================================================================
 * Response Processing
 * ============================================================================ */

/**
 * @brief Process received CAN frame
 *
 * Call this from CAN RX interrupt or polling loop.
 * Parses the frame and invokes the appropriate callback.
 *
 * @param header HAL FDCAN RX header
 * @param data Frame data
 * @param data_len Data length
 */
void moteus_process_rx(const FDCAN_RxHeaderTypeDef* header,
                        const uint8_t* data,
                        uint32_t data_len);

/**
 * @brief HAL FDCAN RX callback helper
 *
 * Convenience function to call from HAL_FDCAN_RxFifo0Callback.
 * Automatically retrieves the frame and processes it.
 *
 * @param hfdcan HAL FDCAN handle
 */
void moteus_fdcan_rx_callback(FDCAN_HandleTypeDef* hfdcan);

/**
 * @brief Check if a response has been received (non-blocking)
 *
 * Use this after sending a command with wait=false.
 * Returns true if new data is available, and clears the flag.
 *
 * @param motor Motor instance
 * @return true if new response available, false otherwise
 */
bool moteus_poll(moteus_motor_t* motor);

/* ============================================================================
 * Motor Registry
 * ============================================================================ */

/**
 * @brief Get motor by ID
 *
 * @param id Motor CAN ID
 * @return Motor instance, or NULL if not found
 */
moteus_motor_t* moteus_get_motor_by_id(uint8_t id);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Check if a float is NaN
 */
static inline bool moteus_is_nan(float value)
{
    return value != value;
}

/**
 * @brief Get mode name string
 *
 * @param mode Mode value
 * @return Mode name string
 */
const char* moteus_mode_str(uint8_t mode);

/**
 * @brief Get fault name string
 *
 * @param fault Fault code
 * @return Fault name string
 */
const char* moteus_fault_str(uint8_t fault);

/* ============================================================================
 * Error Handling
 * ============================================================================ */

/**
 * @brief Get error name string
 *
 * @param error Error code (MOTEUS_ERR_*)
 * @return Error name string
 */
const char* moteus_error_str(int error);

/**
 * @brief Get last error code
 *
 * @param motor Motor instance
 * @return Last error code (MOTEUS_ERR_*), or MOTEUS_OK if no error
 */
int moteus_get_last_error(moteus_motor_t* motor);

/**
 * @brief Get last motor fault code
 *
 * @param motor Motor instance
 * @return Last fault code (MOTEUS_FAULT_*), or MOTEUS_FAULT_NONE
 */
uint8_t moteus_get_last_fault(moteus_motor_t* motor);

/**
 * @brief Clear error state
 *
 * Clears last_error, last_fault, and the response_received flag.
 *
 * @param motor Motor instance
 */
void moteus_clear_error(moteus_motor_t* motor);

/**
 * @brief Enable/disable auto-recovery
 *
 * When enabled, the library will automatically send a stop command
 * when a timeout or motor fault is detected, then retry the command.
 *
 * @param motor Motor instance
 * @param enable true to enable auto-recovery
 */
void moteus_set_auto_recover(moteus_motor_t* motor, bool enable);

/**
 * @brief Check if auto-recovery is enabled
 *
 * @param motor Motor instance
 * @return true if auto-recovery is enabled
 */
bool moteus_get_auto_recover(moteus_motor_t* motor);

/**
 * @brief Attempt to recover from error state
 *
 * Sends a stop command to clear faults and reset motor state.
 * Updates recovery_count on success.
 *
 * @param motor Motor instance
 * @return Pointer to result on success, NULL on failure
 */
const moteus_result_t* moteus_recover(moteus_motor_t* motor);

/**
 * @brief Check motor status and return any fault
 *
 * Queries the motor and checks for fault/timeout modes.
 * Sets last_error and last_fault if a problem is detected.
 *
 * @param motor Motor instance
 * @return MOTEUS_OK if healthy, MOTEUS_ERR_MOTOR_FAULT or MOTEUS_ERR_MOTOR_TIMEOUT if not
 */
int moteus_check_status(moteus_motor_t* motor);

/**
 * @brief Check if motor is in a fault or timeout state
 *
 * Uses cached result, does not query motor.
 *
 * @param motor Motor instance
 * @return true if motor is in fault or timeout mode
 */
bool moteus_is_error_state(moteus_motor_t* motor);

#ifdef __cplusplus
}
#endif

#endif /* MOTEUS_H */
