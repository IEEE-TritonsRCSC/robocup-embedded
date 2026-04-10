/**
 * @file moteus_types.h
 * @brief Data structures and enums for Moteus library
 *
 * Contains all type definitions used throughout the Moteus STM32 library.
 */

#ifndef MOTEUS_TYPES_H
#define MOTEUS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Resolution Types
 * ============================================================================ */

/**
 * @brief Data resolution for register encoding/decoding
 *
 * Determines how values are encoded in CAN frames.
 * Higher resolutions use more bytes but provide greater precision.
 */
typedef enum {
    MOTEUS_RES_IGNORE = 0,  /**< Don't include this register */
    MOTEUS_RES_INT8   = 1,  /**< 8-bit signed integer (1 byte) */
    MOTEUS_RES_INT16  = 2,  /**< 16-bit signed integer (2 bytes) */
    MOTEUS_RES_INT32  = 3,  /**< 32-bit signed integer (4 bytes) */
    MOTEUS_RES_FLOAT  = 4   /**< 32-bit IEEE 754 float (4 bytes) */
} moteus_resolution_t;

/* ============================================================================
 * Query Configuration
 * ============================================================================ */

/**
 * @brief Query format configuration
 *
 * Specifies which registers to query and at what resolution.
 * Set resolution to MOTEUS_RES_IGNORE to skip a register.
 */
typedef struct {
    moteus_resolution_t mode;        /**< Controller mode (usually INT8) */
    moteus_resolution_t position;    /**< Position in revolutions */
    moteus_resolution_t velocity;    /**< Velocity in rev/s */
    moteus_resolution_t torque;      /**< Torque in Nm */
    moteus_resolution_t q_current;   /**< Q-axis current in A */
    moteus_resolution_t d_current;   /**< D-axis current in A */
    moteus_resolution_t voltage;     /**< Bus voltage in V */
    moteus_resolution_t temperature; /**< Board temperature in C */
    moteus_resolution_t fault;       /**< Fault code */
} moteus_query_format_t;

/* Alias for compatibility */
typedef moteus_query_format_t moteus_query_resolution_t;

/**
 * @brief Default query format (matches official moteus library)
 *
 * Uses FLOAT for position/velocity/torque to match the official Python library.
 */
#define MOTEUS_QUERY_FORMAT_DEFAULT { \
    .mode = MOTEUS_RES_INT8,          \
    .position = MOTEUS_RES_FLOAT,     \
    .velocity = MOTEUS_RES_FLOAT,     \
    .torque = MOTEUS_RES_FLOAT,       \
    .q_current = MOTEUS_RES_IGNORE,   \
    .d_current = MOTEUS_RES_IGNORE,   \
    .voltage = MOTEUS_RES_INT8,       \
    .temperature = MOTEUS_RES_INT8,   \
    .fault = MOTEUS_RES_INT8          \
}

#define MOTEUS_QUERY_RESOLUTION_DEFAULT MOTEUS_QUERY_FORMAT_DEFAULT

/* ============================================================================
 * Command Structures
 * ============================================================================ */

/**
 * @brief Position command parameters
 *
 * All fields support NaN to indicate "don't change" or "use default".
 */
typedef struct {
    float position;           /**< Target position in revolutions (NaN = don't change) */
    float velocity;           /**< Target velocity in rev/s (NaN = use trajectory) */
    float feedforward_torque; /**< Feedforward torque in Nm (NaN = 0) */
    float kp_scale;           /**< Position gain scale 0-1 (NaN = 1.0) */
    float kd_scale;           /**< Velocity gain scale 0-1 (NaN = 1.0) */
    float max_torque;         /**< Maximum torque in Nm (NaN = use configured) */
    float stop_position;      /**< Stop position in revolutions (NaN = ignore) */
    float watchdog_timeout;   /**< Watchdog timeout in seconds (NaN = use configured) */
    float velocity_limit;     /**< Velocity limit in rev/s (NaN = use configured) */
    float accel_limit;        /**< Acceleration limit in rev/s^2 (NaN = use configured) */
} moteus_position_cmd_t;

/**
 * @brief Default position command (position-only, full gains)
 */
#define MOTEUS_POSITION_CMD_DEFAULT { \
    .position = 0.0f,                 \
    .velocity = __builtin_nanf(""),   \
    .feedforward_torque = 0.0f,       \
    .kp_scale = 1.0f,                 \
    .kd_scale = 1.0f,                 \
    .max_torque = __builtin_nanf(""), \
    .stop_position = __builtin_nanf(""),    \
    .watchdog_timeout = __builtin_nanf(""), \
    .velocity_limit = __builtin_nanf(""),   \
    .accel_limit = __builtin_nanf("")       \
}

/**
 * @brief Resolution configuration for position commands
 */
typedef struct {
    moteus_resolution_t position;
    moteus_resolution_t velocity;
    moteus_resolution_t feedforward_torque;
    moteus_resolution_t kp_scale;
    moteus_resolution_t kd_scale;
    moteus_resolution_t max_torque;
    moteus_resolution_t stop_position;
    moteus_resolution_t watchdog_timeout;
    moteus_resolution_t velocity_limit;
    moteus_resolution_t accel_limit;
} moteus_position_resolution_t;

/**
 * @brief Default position command resolution (float for all)
 */
#define MOTEUS_POSITION_RESOLUTION_DEFAULT { \
    .position = MOTEUS_RES_FLOAT,            \
    .velocity = MOTEUS_RES_FLOAT,            \
    .feedforward_torque = MOTEUS_RES_FLOAT,  \
    .kp_scale = MOTEUS_RES_FLOAT,            \
    .kd_scale = MOTEUS_RES_FLOAT,            \
    .max_torque = MOTEUS_RES_FLOAT,          \
    .stop_position = MOTEUS_RES_IGNORE,      \
    .watchdog_timeout = MOTEUS_RES_IGNORE,   \
    .velocity_limit = MOTEUS_RES_IGNORE,     \
    .accel_limit = MOTEUS_RES_IGNORE         \
}

/**
 * @brief Stay-within command parameters
 *
 * Keeps motor within bounded position range.
 */
typedef struct {
    float lower_bound;        /**< Lower position bound in revolutions */
    float upper_bound;        /**< Upper position bound in revolutions */
    float feedforward_torque; /**< Feedforward torque in Nm (NaN = 0) */
    float kp_scale;           /**< Position gain scale 0-1 (NaN = 1.0) */
    float kd_scale;           /**< Velocity gain scale 0-1 (NaN = 1.0) */
    float max_torque;         /**< Maximum torque in Nm (NaN = use configured) */
    float watchdog_timeout;   /**< Watchdog timeout in seconds (NaN = use configured) */
    float velocity_limit;     /**< Velocity limit in rev/s (NaN = use configured) */
} moteus_stay_within_cmd_t;

/**
 * @brief Default stay-within command
 */
#define MOTEUS_STAY_WITHIN_CMD_DEFAULT { \
    .lower_bound = -1.0f,                \
    .upper_bound = 1.0f,                 \
    .feedforward_torque = 0.0f,          \
    .kp_scale = 1.0f,                    \
    .kd_scale = 1.0f,                    \
    .max_torque = __builtin_nanf(""),    \
    .watchdog_timeout = __builtin_nanf(""), \
    .velocity_limit = __builtin_nanf("") \
}

/**
 * @brief Resolution configuration for stay-within commands
 */
typedef struct {
    moteus_resolution_t lower_bound;
    moteus_resolution_t upper_bound;
    moteus_resolution_t feedforward_torque;
    moteus_resolution_t kp_scale;
    moteus_resolution_t kd_scale;
    moteus_resolution_t max_torque;
    moteus_resolution_t watchdog_timeout;
    moteus_resolution_t velocity_limit;
} moteus_stay_within_resolution_t;

/**
 * @brief Default stay-within command resolution
 */
#define MOTEUS_STAY_WITHIN_RESOLUTION_DEFAULT { \
    .lower_bound = MOTEUS_RES_FLOAT,            \
    .upper_bound = MOTEUS_RES_FLOAT,            \
    .feedforward_torque = MOTEUS_RES_FLOAT,     \
    .kp_scale = MOTEUS_RES_FLOAT,               \
    .kd_scale = MOTEUS_RES_FLOAT,               \
    .max_torque = MOTEUS_RES_IGNORE,            \
    .watchdog_timeout = MOTEUS_RES_IGNORE,      \
    .velocity_limit = MOTEUS_RES_IGNORE         \
}

/**
 * @brief VFOC (Voltage Field-Oriented Control) command parameters
 *
 * Direct voltage control with specified electrical angle.
 */
typedef struct {
    float theta;              /**< Electrical angle in radians/pi (1.0 = 180 degrees) */
    float voltage;            /**< Voltage magnitude in V */
    float theta_rate;         /**< Rate of theta change in rad/pi/s (NaN = 0) */
} moteus_vfoc_cmd_t;

/**
 * @brief Default VFOC command
 */
#define MOTEUS_VFOC_CMD_DEFAULT { \
    .theta = 0.0f,                \
    .voltage = 0.0f,              \
    .theta_rate = __builtin_nanf("") \
}

/**
 * @brief Resolution configuration for VFOC commands
 */
typedef struct {
    moteus_resolution_t theta;
    moteus_resolution_t voltage;
    moteus_resolution_t theta_rate;
} moteus_vfoc_resolution_t;

/**
 * @brief Default VFOC command resolution
 */
#define MOTEUS_VFOC_RESOLUTION_DEFAULT { \
    .theta = MOTEUS_RES_FLOAT,           \
    .voltage = MOTEUS_RES_FLOAT,         \
    .theta_rate = MOTEUS_RES_IGNORE      \
}

/**
 * @brief Output position set command
 *
 * Used for rezero operations.
 */
typedef struct {
    float position;           /**< Position value to set in revolutions */
} moteus_output_position_cmd_t;

/**
 * @brief Default output position command
 */
#define MOTEUS_OUTPUT_POSITION_CMD_DEFAULT { \
    .position = 0.0f                         \
}

/**
 * @brief GPIO command parameters
 */
typedef struct {
    uint8_t aux1;             /**< AUX1 GPIO output bits (bit 0-3 = pins 1-4) */
    uint8_t aux2;             /**< AUX2 GPIO output bits (bit 0-3 = pins 1-4) */
} moteus_gpio_cmd_t;

/**
 * @brief Default GPIO command (all outputs low)
 */
#define MOTEUS_GPIO_CMD_DEFAULT { \
    .aux1 = 0,                    \
    .aux2 = 0                     \
}

/**
 * @brief GPIO status result
 */
typedef struct {
    uint8_t aux1;             /**< AUX1 GPIO input bits */
    uint8_t aux2;             /**< AUX2 GPIO input bits */
} moteus_gpio_result_t;

/**
 * @brief Analog input result
 */
typedef struct {
    float aux1[5];            /**< AUX1 analog inputs (0-1 range) */
    float aux2[5];            /**< AUX2 analog inputs (0-1 range) */
} moteus_analog_result_t;

/**
 * @brief Diagnostic read/write command
 *
 * For reading/writing configuration parameters.
 */
typedef struct {
    uint8_t channel;          /**< Diagnostic channel (1=config, 2=telemetry) */
    uint8_t data[60];         /**< Data payload */
    uint8_t len;              /**< Data length */
} moteus_diagnostic_cmd_t;

/**
 * @brief Diagnostic response
 */
typedef struct {
    uint8_t channel;          /**< Diagnostic channel */
    uint8_t data[60];         /**< Response data */
    uint8_t len;              /**< Data length */
} moteus_diagnostic_result_t;

/* ============================================================================
 * Result Structure
 * ============================================================================ */

/**
 * @brief Motor state/response data
 *
 * Populated from query responses. Fields that weren't queried will be NaN.
 */
typedef struct {
    uint8_t mode;             /**< Current controller mode (MOTEUS_MODE_*) */
    uint8_t fault;            /**< Fault code (MOTEUS_FAULT_*) */
    float position;           /**< Current position in revolutions */
    float velocity;           /**< Current velocity in rev/s */
    float torque;             /**< Current torque in Nm */
    float q_current;          /**< Q-axis current in A */
    float d_current;          /**< D-axis current in A */
    float voltage;            /**< Bus voltage in V */
    float temperature;        /**< Board temperature in C */
    float motor_temperature;  /**< Motor temperature in C */
    float power;              /**< Electrical power in W */
    float abs_position;       /**< Absolute position in revolutions */
    uint8_t rezero_state;     /**< Rezero/home state */
    bool trajectory_complete; /**< True if trajectory is complete */
} moteus_result_t;

/**
 * @brief Initialize result struct with default (NaN) values
 */
#define MOTEUS_RESULT_INIT { \
    .mode = 0,               \
    .fault = 0,              \
    .position = __builtin_nanf(""),       \
    .velocity = __builtin_nanf(""),       \
    .torque = __builtin_nanf(""),         \
    .q_current = __builtin_nanf(""),      \
    .d_current = __builtin_nanf(""),      \
    .voltage = __builtin_nanf(""),        \
    .temperature = __builtin_nanf(""),    \
    .motor_temperature = __builtin_nanf(""), \
    .power = __builtin_nanf(""),          \
    .abs_position = __builtin_nanf(""),   \
    .rezero_state = 0,                    \
    .trajectory_complete = false          \
}

/* ============================================================================
 * CAN Frame Structures
 * ============================================================================ */

/**
 * @brief Raw CAN frame for TX/RX
 */
typedef struct {
    uint32_t id;         /**< CAN identifier (29-bit extended) */
    uint8_t data[64];    /**< Frame data (up to 64 bytes for CAN-FD) */
    uint8_t len;         /**< Data length */
    bool is_fd;          /**< True for CAN-FD frame */
    bool brs;            /**< Bit rate switch (use faster data rate) */
} moteus_can_frame_t;

/* ============================================================================
 * Motor Instance (Forward Declaration)
 * ============================================================================ */

/* Full definition in moteus.h */
typedef struct moteus_motor moteus_motor_t;

/**
 * @brief Response callback function type
 *
 * Called when a response is received from the motor.
 *
 * @param motor The motor instance that received the response
 * @param result Decoded motor state
 * @param user_data User-provided context pointer
 */
typedef void (*moteus_response_cb_t)(moteus_motor_t* motor,
                                      const moteus_result_t* result,
                                      void* user_data);

/**
 * @brief Error callback function type
 *
 * Called when a communication error occurs.
 *
 * @param motor The motor instance (may be NULL for bus-level errors)
 * @param error_code Error code
 * @param user_data User-provided context pointer
 */
typedef void (*moteus_error_cb_t)(moteus_motor_t* motor,
                                   int error_code,
                                   void* user_data);

/* ============================================================================
 * Error Codes
 * ============================================================================ */

#define MOTEUS_OK                   0
#define MOTEUS_ERR_INVALID_PARAM   -1
#define MOTEUS_ERR_BUFFER_FULL     -2
#define MOTEUS_ERR_TX_FAILED       -3
#define MOTEUS_ERR_TIMEOUT         -4
#define MOTEUS_ERR_INVALID_FRAME   -5
#define MOTEUS_ERR_NO_MOTOR        -6
#define MOTEUS_ERR_NOT_INITIALIZED -7
#define MOTEUS_ERR_DIAGNOSTIC      -8
#define MOTEUS_ERR_MOTOR_FAULT     -9   /**< Motor reported a fault */
#define MOTEUS_ERR_MOTOR_TIMEOUT   -10  /**< Motor watchdog triggered */
#define MOTEUS_ERR_CAN_BUS         -11  /**< CAN bus error */
#define MOTEUS_ERR_RECOVERED       -12  /**< Error occurred but auto-recovered */

/* Motor modes and fault codes are defined in moteus_registers.h */

#ifdef __cplusplus
}
#endif

#endif /* MOTEUS_TYPES_H */
