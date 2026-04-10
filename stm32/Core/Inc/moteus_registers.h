/**
 * @file moteus_registers.h
 * @brief Moteus register definitions and scaling constants
 *
 * Register addresses and scaling factors for the Moteus motor controller
 * CAN protocol. Based on the official Moteus CAN protocol documentation.
 *
 * @see https://github.com/mjbots/moteus/blob/main/docs/reference.md
 */

#ifndef MOTEUS_REGISTERS_H
#define MOTEUS_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Query Registers (Read from controller) - 0x000 to 0x0FF
 * ============================================================================ */

/* Basic status registers */
#define MOTEUS_REG_Q_MODE                   0x000
#define MOTEUS_REG_Q_POSITION               0x001
#define MOTEUS_REG_Q_VELOCITY               0x002
#define MOTEUS_REG_Q_TORQUE                 0x003
#define MOTEUS_REG_Q_Q_CURRENT              0x004
#define MOTEUS_REG_Q_D_CURRENT              0x005
#define MOTEUS_REG_Q_ABS_POSITION           0x006
#define MOTEUS_REG_Q_POWER                  0x007
#define MOTEUS_REG_Q_MOTOR_TEMPERATURE      0x00A
#define MOTEUS_REG_Q_TRAJECTORY_COMPLETE    0x00B
#define MOTEUS_REG_Q_REZERO_STATE           0x00C
#define MOTEUS_REG_Q_HOME_STATE             0x00C  /* Alias */
#define MOTEUS_REG_Q_VOLTAGE                0x00D
#define MOTEUS_REG_Q_TEMPERATURE            0x00E
#define MOTEUS_REG_Q_FAULT                  0x00F

/* PWM output registers */
#define MOTEUS_REG_Q_PWM_PHASE_A            0x010
#define MOTEUS_REG_Q_PWM_PHASE_B            0x011
#define MOTEUS_REG_Q_PWM_PHASE_C            0x012

/* Voltage measurement registers */
#define MOTEUS_REG_Q_VOLTAGE_PHASE_A        0x014
#define MOTEUS_REG_Q_VOLTAGE_PHASE_B        0x015
#define MOTEUS_REG_Q_VOLTAGE_PHASE_C        0x016

/* VFOC state registers */
#define MOTEUS_REG_Q_VFOC_THETA             0x018
#define MOTEUS_REG_Q_VFOC_VOLTAGE           0x019
#define MOTEUS_REG_Q_VFOC_D_VOLTAGE         0x01A
#define MOTEUS_REG_Q_VFOC_Q_VOLTAGE         0x01B

/* Current command registers (readable) */
#define MOTEUS_REG_Q_CMD_Q_CURRENT          0x01C
#define MOTEUS_REG_Q_CMD_D_CURRENT          0x01D

/* Position/velocity command registers (readable) */
#define MOTEUS_REG_Q_VFOC_THETA_RATE        0x01E
#define MOTEUS_REG_Q_CMD_POSITION           0x020
#define MOTEUS_REG_Q_CMD_VELOCITY           0x021

/* Control loop internal state */
#define MOTEUS_REG_Q_POSITION_KP            0x030
#define MOTEUS_REG_Q_POSITION_KI            0x031
#define MOTEUS_REG_Q_POSITION_KD            0x032
#define MOTEUS_REG_Q_POSITION_FEEDFORWARD   0x033
#define MOTEUS_REG_Q_POSITION_COMMAND       0x034

#define MOTEUS_REG_Q_CONTROL_POSITION       0x038
#define MOTEUS_REG_Q_CONTROL_VELOCITY       0x039
#define MOTEUS_REG_Q_CONTROL_TORQUE         0x03A
#define MOTEUS_REG_Q_CONTROL_Q_CURRENT      0x03B
#define MOTEUS_REG_Q_CONTROL_D_CURRENT      0x03C
#define MOTEUS_REG_Q_CONTROL_ABS_POSITION   0x03D

#define MOTEUS_REG_Q_POSITION_ERROR         0x03F
#define MOTEUS_REG_Q_VELOCITY_ERROR         0x040
#define MOTEUS_REG_Q_TORQUE_ERROR           0x041

/* External encoder registers */
#define MOTEUS_REG_Q_ENCODER_0_POSITION     0x050
#define MOTEUS_REG_Q_ENCODER_0_VELOCITY     0x051
#define MOTEUS_REG_Q_ENCODER_1_POSITION     0x052
#define MOTEUS_REG_Q_ENCODER_1_VELOCITY     0x053
#define MOTEUS_REG_Q_ENCODER_2_POSITION     0x054
#define MOTEUS_REG_Q_ENCODER_2_VELOCITY     0x055
#define MOTEUS_REG_Q_ENCODER_VALIDITY       0x058

/* GPIO status registers */
#define MOTEUS_REG_Q_AUX1_GPIO_STATUS       0x05C
#define MOTEUS_REG_Q_AUX2_GPIO_STATUS       0x05D

/* Analog input registers - AUX1 */
#define MOTEUS_REG_Q_AUX1_ANALOG_IN1        0x060
#define MOTEUS_REG_Q_AUX1_ANALOG_IN2        0x061
#define MOTEUS_REG_Q_AUX1_ANALOG_IN3        0x062
#define MOTEUS_REG_Q_AUX1_ANALOG_IN4        0x063
#define MOTEUS_REG_Q_AUX1_ANALOG_IN5        0x064

/* Analog input registers - AUX2 */
#define MOTEUS_REG_Q_AUX2_ANALOG_IN1        0x068
#define MOTEUS_REG_Q_AUX2_ANALOG_IN2        0x069
#define MOTEUS_REG_Q_AUX2_ANALOG_IN3        0x06A
#define MOTEUS_REG_Q_AUX2_ANALOG_IN4        0x06B
#define MOTEUS_REG_Q_AUX2_ANALOG_IN5        0x06C

/* Timing registers */
#define MOTEUS_REG_Q_MILLISECOND_COUNTER    0x070
#define MOTEUS_REG_Q_CLOCK_TRIM             0x071

/* System identification */
#define MOTEUS_REG_Q_REGISTER_MAP_VERSION   0x102
#define MOTEUS_REG_Q_SERIAL_NUMBER_1        0x120
#define MOTEUS_REG_Q_SERIAL_NUMBER_2        0x121
#define MOTEUS_REG_Q_SERIAL_NUMBER_3        0x122

/* Hardware diagnostics */
#define MOTEUS_REG_Q_DRIVER_FAULT1          0x140
#define MOTEUS_REG_Q_DRIVER_FAULT2          0x141

/* ============================================================================
 * Command Registers (Write to controller)
 * ============================================================================ */

/* Mode register */
#define MOTEUS_REG_MODE                     0x000

/* VFOC command registers */
#define MOTEUS_REG_VFOC_THETA               0x018
#define MOTEUS_REG_VFOC_VOLTAGE             0x019
#define MOTEUS_REG_VFOC_THETA_RATE          0x01E

/* Current command registers */
#define MOTEUS_REG_CMD_Q_CURRENT            0x01C
#define MOTEUS_REG_CMD_D_CURRENT            0x01D

/* Position command registers */
#define MOTEUS_REG_POSITION                 0x020
#define MOTEUS_REG_VELOCITY                 0x021
#define MOTEUS_REG_FEEDFORWARD_TORQUE       0x022
#define MOTEUS_REG_KP_SCALE                 0x023
#define MOTEUS_REG_KD_SCALE                 0x024
#define MOTEUS_REG_MAX_TORQUE               0x025
#define MOTEUS_REG_COMMANDED_STOP_POSITION  0x026
#define MOTEUS_REG_WATCHDOG_TIMEOUT         0x027
#define MOTEUS_REG_VELOCITY_LIMIT           0x028
#define MOTEUS_REG_ACCEL_LIMIT              0x029
#define MOTEUS_REG_FIXED_VOLTAGE_OVERRIDE   0x02A
#define MOTEUS_REG_ILIMIT_SCALE             0x02B

/* Stay-within command registers */
#define MOTEUS_REG_STAY_WITHIN_LOWER        0x040
#define MOTEUS_REG_STAY_WITHIN_UPPER        0x041
#define MOTEUS_REG_STAY_WITHIN_FF_TORQUE    0x042
#define MOTEUS_REG_STAY_WITHIN_KP_SCALE     0x043
#define MOTEUS_REG_STAY_WITHIN_KD_SCALE     0x044
#define MOTEUS_REG_STAY_WITHIN_MAX_TORQUE   0x045
#define MOTEUS_REG_STAY_WITHIN_TIMEOUT      0x046
#define MOTEUS_REG_STAY_WITHIN_VELOCITY_LIMIT 0x047

/* GPIO command registers */
#define MOTEUS_REG_AUX1_GPIO_COMMAND        0x05C
#define MOTEUS_REG_AUX2_GPIO_COMMAND        0x05D

/* Clock trim */
#define MOTEUS_REG_CLOCK_TRIM               0x071

/* Calibration/rezero command registers */
#define MOTEUS_REG_SET_OUTPUT_NEAREST       0x130
#define MOTEUS_REG_REZERO                   0x130  /* Alias */
#define MOTEUS_REG_SET_OUTPUT_EXACT         0x131
#define MOTEUS_REG_REQUIRE_REINDEX          0x132
#define MOTEUS_REG_RECAPTURE_POSITION_VEL   0x133

/* ============================================================================
 * Controller Modes
 * ============================================================================ */

#define MOTEUS_MODE_STOPPED                 0
#define MOTEUS_MODE_FAULT                   1
#define MOTEUS_MODE_ENABLING                2
#define MOTEUS_MODE_CALIBRATING             3
#define MOTEUS_MODE_CALIBRATION_COMPLETE    4
#define MOTEUS_MODE_PWM                     5
#define MOTEUS_MODE_VOLTAGE                 6
#define MOTEUS_MODE_VOLTAGE_FOC             7
#define MOTEUS_MODE_VOLTAGE_DQ              8
#define MOTEUS_MODE_CURRENT                 9
#define MOTEUS_MODE_POSITION                10
#define MOTEUS_MODE_TIMEOUT                 11
#define MOTEUS_MODE_ZERO_VELOCITY           12
#define MOTEUS_MODE_STAY_WITHIN             13
#define MOTEUS_MODE_MEASURE_IND             14
#define MOTEUS_MODE_BRAKE                   15

/* ============================================================================
 * Fault Codes
 * ============================================================================ */

#define MOTEUS_FAULT_NONE                   0
#define MOTEUS_FAULT_DMA_ERROR              1
#define MOTEUS_FAULT_ENCODER_NOT_FOUND      2
#define MOTEUS_FAULT_MOTOR_NOT_CONFIGURED   3
#define MOTEUS_FAULT_PWM_CYCLE_OVERRUN      4
#define MOTEUS_FAULT_OVER_VOLTAGE           5
#define MOTEUS_FAULT_ENCODER_FAULT          6
#define MOTEUS_FAULT_MOTOR_DRIVER_FAULT     7
#define MOTEUS_FAULT_OVER_TEMP              8
#define MOTEUS_FAULT_START_OUT_POSITION     9
#define MOTEUS_FAULT_UNDER_VOLTAGE          10
#define MOTEUS_FAULT_CONFIG_CHANGED         11
#define MOTEUS_FAULT_MOTOR_DRIVER_ENABLE    12
#define MOTEUS_FAULT_MOTOR_DRIVER_FAULT2    13

/* ============================================================================
 * Rezero State
 * ============================================================================ */

#define MOTEUS_REZERO_NOT_STARTED           0
#define MOTEUS_REZERO_IN_PROGRESS           1
#define MOTEUS_REZERO_COMPLETE              2

/* ============================================================================
 * Diagnostic Channel IDs
 * ============================================================================ */

#define MOTEUS_DIAG_CHANNEL_CONFIG          1   /* Configuration read/write */
#define MOTEUS_DIAG_CHANNEL_TELEMETRY       2   /* Telemetry stream */

/* ============================================================================
 * Scaling Constants
 *
 * These define the fixed-point scaling used for int8/int16 encoding.
 * Float values are transmitted as-is (IEEE 754).
 * ============================================================================ */

/* Position: revolutions */
#define MOTEUS_SCALE_POSITION_INT8          0.01f       /* ±1.27 rev */
#define MOTEUS_SCALE_POSITION_INT16         0.0001f     /* ±3.2767 rev */
#define MOTEUS_SCALE_POSITION_INT32         0.00001f    /* ±21474.83647 rev */

/* Velocity: revolutions per second */
#define MOTEUS_SCALE_VELOCITY_INT8          0.1f        /* ±12.7 rev/s */
#define MOTEUS_SCALE_VELOCITY_INT16         0.00025f    /* ±8.19 rev/s */
#define MOTEUS_SCALE_VELOCITY_INT32         0.00001f    /* ±21474.83647 rev/s */

/* Torque: Newton-meters */
#define MOTEUS_SCALE_TORQUE_INT8            0.5f        /* ±63.5 Nm */
#define MOTEUS_SCALE_TORQUE_INT16           0.01f       /* ±327.67 Nm */
#define MOTEUS_SCALE_TORQUE_INT32           0.001f      /* ±2147483.647 Nm */

/* Current: Amps */
#define MOTEUS_SCALE_CURRENT_INT8           1.0f        /* ±127 A */
#define MOTEUS_SCALE_CURRENT_INT16          0.1f        /* ±3276.7 A */
#define MOTEUS_SCALE_CURRENT_INT32          0.001f      /* ±2147483.647 A */

/* Voltage: Volts */
#define MOTEUS_SCALE_VOLTAGE_INT8           0.5f        /* ±63.5 V */
#define MOTEUS_SCALE_VOLTAGE_INT16          0.1f        /* ±3276.7 V */
#define MOTEUS_SCALE_VOLTAGE_INT32          0.001f      /* ±2147483.647 V */

/* Temperature: Celsius */
#define MOTEUS_SCALE_TEMPERATURE_INT8       1.0f        /* ±127 C */
#define MOTEUS_SCALE_TEMPERATURE_INT16      0.1f        /* ±3276.7 C */
#define MOTEUS_SCALE_TEMPERATURE_INT32      0.001f      /* ±2147483.647 C */

/* PWM: duty cycle (-1.0 to 1.0) */
#define MOTEUS_SCALE_PWM_INT8               0.01f       /* ±1.27 */
#define MOTEUS_SCALE_PWM_INT16              0.001f      /* ±32.767 */
#define MOTEUS_SCALE_PWM_INT32              0.00001f    /* ±21474.83647 */

/* KP/KD scale: 0-1 */
#define MOTEUS_SCALE_KP_KD_INT8             0.01f       /* 0-1.27 */
#define MOTEUS_SCALE_KP_KD_INT16            0.0001f     /* 0-3.2767 */
#define MOTEUS_SCALE_KP_KD_INT32            0.00001f    /* 0-21474.83647 */

/* Time: seconds */
#define MOTEUS_SCALE_TIME_INT8              0.01f       /* ±1.27 s */
#define MOTEUS_SCALE_TIME_INT16             0.001f      /* ±32.767 s */
#define MOTEUS_SCALE_TIME_INT32             0.000001f   /* ±2147.483647 s */

/* Acceleration: rev/s^2 */
#define MOTEUS_SCALE_ACCEL_INT8             0.05f       /* ±6.35 rev/s^2 */
#define MOTEUS_SCALE_ACCEL_INT16            0.001f      /* ±32.767 rev/s^2 */
#define MOTEUS_SCALE_ACCEL_INT32            0.00001f    /* ±21474.83647 rev/s^2 */

/* Power: Watts */
#define MOTEUS_SCALE_POWER_INT8             10.0f       /* ±1270 W */
#define MOTEUS_SCALE_POWER_INT16            0.05f       /* ±1638.35 W */
#define MOTEUS_SCALE_POWER_INT32            0.0001f     /* ±214748.3647 W */

/* VFOC theta: radians/pi (so 1.0 = pi radians = 180 degrees) */
#define MOTEUS_SCALE_THETA_INT8             0.01f       /* ±1.27 * pi rad */
#define MOTEUS_SCALE_THETA_INT16            0.0001f     /* ±3.2767 * pi rad */
#define MOTEUS_SCALE_THETA_INT32            0.00001f    /* ±21474 * pi rad */

/* ============================================================================
 * Special Values
 * ============================================================================ */

/* Use NaN for "don't change" / "ignore" in position commands */
#define MOTEUS_NAN                          __builtin_nanf("")

#ifdef __cplusplus
}
#endif

#endif /* MOTEUS_REGISTERS_H */
