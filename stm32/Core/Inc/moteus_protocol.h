/**
 * @file moteus_protocol.h
 * @brief Moteus CAN protocol encoding/decoding
 *
 * Low-level protocol implementation for building and parsing
 * Moteus CAN frames. Based on the multiplex protocol specification.
 *
 * @see https://github.com/mjbots/mjlib/blob/main/mjlib/multiplex/format.h
 */

#ifndef MOTEUS_PROTOCOL_H
#define MOTEUS_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "moteus_types.h"
#include "moteus_registers.h"
#include "moteus.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CAN ID Format (29-bit extended ID)
 *
 * Bits 28-16: CAN prefix (default 0x0000)
 * Bit 15:     Reply requested flag (0x8000)
 * Bits 14-8:  Source ID
 * Bits 7-0:   Destination servo ID
 * ============================================================================ */

#define MOTEUS_CAN_PREFIX           0x0000
#define MOTEUS_CAN_REPLY_FLAG       0x8000
#define MOTEUS_CAN_SOURCE_SHIFT     8
#define MOTEUS_CAN_DEST_MASK        0xFF

/**
 * @brief Build a CAN ID for sending to a motor
 *
 * @param dest_id Destination motor ID (0-127)
 * @param source_id Source ID (usually 0)
 * @param request_reply True to request a reply frame
 * @return 29-bit extended CAN ID
 */
static inline uint32_t moteus_make_can_id(uint8_t dest_id, uint8_t source_id, bool request_reply)
{
    uint32_t id = (MOTEUS_CAN_PREFIX << 16) |
                  ((uint32_t)source_id << MOTEUS_CAN_SOURCE_SHIFT) |
                  dest_id;
    if (request_reply) {
        id |= MOTEUS_CAN_REPLY_FLAG;
    }
    return id;
}

/**
 * @brief Extract destination ID from CAN ID
 */
static inline uint8_t moteus_get_dest_id(uint32_t can_id)
{
    return can_id & MOTEUS_CAN_DEST_MASK;
}

/**
 * @brief Extract source ID from CAN ID
 */
static inline uint8_t moteus_get_source_id(uint32_t can_id)
{
    return (can_id >> MOTEUS_CAN_SOURCE_SHIFT) & 0x7F;
}

/**
 * @brief Check if CAN ID requests a reply
 */
static inline bool moteus_is_reply_requested(uint32_t can_id)
{
    return (can_id & MOTEUS_CAN_REPLY_FLAG) != 0;
}

/* ============================================================================
 * Multiplex Protocol Opcodes
 *
 * Format: 0bTTNNNNNN
 *   TT: Type (00=int8, 01=int16, 10=int32, 11=float)
 *   NNNNNN: Number of registers (0 = special, 1-63 = count)
 *
 * Write opcodes: 0x00-0x0F (write N registers starting at next byte)
 * Read opcodes:  0x10-0x1F (read N registers starting at next byte)
 * Reply opcodes: 0x20-0x2F (reply with N register values)
 * Error/NOP:     0x50 (NOP), 0x51-0x5F (errors)
 * ============================================================================ */

/* Write registers (n = count 1-3 in LSBs, n=4+ needs varuint) */
#define MOTEUS_OP_WRITE_INT8(n)     (0x00 | ((n) & 0x03))
#define MOTEUS_OP_WRITE_INT16(n)    (0x04 | ((n) & 0x03))
#define MOTEUS_OP_WRITE_INT32(n)    (0x08 | ((n) & 0x03))
#define MOTEUS_OP_WRITE_FLOAT(n)    (0x0C | ((n) & 0x03))

/* Read registers (n = count 1-3 in LSBs, n=4+ needs varuint) */
#define MOTEUS_OP_READ_INT8(n)      (0x10 | ((n) & 0x03))
#define MOTEUS_OP_READ_INT16(n)     (0x14 | ((n) & 0x03))
#define MOTEUS_OP_READ_INT32(n)     (0x18 | ((n) & 0x03))
#define MOTEUS_OP_READ_FLOAT(n)     (0x1C | ((n) & 0x03))

/* Reply with register values */
#define MOTEUS_OP_REPLY_INT8(n)     (0x20 | ((n) & 0x03))
#define MOTEUS_OP_REPLY_INT16(n)    (0x24 | ((n) & 0x03))
#define MOTEUS_OP_REPLY_INT32(n)    (0x28 | ((n) & 0x03))
#define MOTEUS_OP_REPLY_FLOAT(n)    (0x2C | ((n) & 0x03))

/* Write error */
#define MOTEUS_OP_WRITE_ERROR       0x30

/* Read error */
#define MOTEUS_OP_READ_ERROR        0x31

/* NOP */
#define MOTEUS_OP_NOP               0x50

/* Diagnostic stream write (client -> server) */
#define MOTEUS_OP_CLIENT_TO_SERVER  0x40

/* Diagnostic stream read (server -> client) */
#define MOTEUS_OP_SERVER_TO_CLIENT  0x41

/* Client poll (used to request diagnostic response) */
#define MOTEUS_OP_CLIENT_POLL       0x42

/* ============================================================================
 * Encoding Helpers
 * ============================================================================ */

/**
 * @brief Get the byte size for a resolution type
 */
static inline size_t moteus_resolution_size(moteus_resolution_t res)
{
    switch (res) {
        case MOTEUS_RES_INT8:  return 1;
        case MOTEUS_RES_INT16: return 2;
        case MOTEUS_RES_INT32: return 4;
        case MOTEUS_RES_FLOAT: return 4;
        default:               return 0;
    }
}

/**
 * @brief Get the write opcode for a resolution type
 */
static inline uint8_t moteus_write_opcode(moteus_resolution_t res, uint8_t count)
{
    switch (res) {
        case MOTEUS_RES_INT8:  return MOTEUS_OP_WRITE_INT8(count);
        case MOTEUS_RES_INT16: return MOTEUS_OP_WRITE_INT16(count);
        case MOTEUS_RES_INT32: return MOTEUS_OP_WRITE_INT32(count);
        case MOTEUS_RES_FLOAT: return MOTEUS_OP_WRITE_FLOAT(count);
        default:               return MOTEUS_OP_NOP;
    }
}

/**
 * @brief Get the read opcode for a resolution type
 */
static inline uint8_t moteus_read_opcode(moteus_resolution_t res, uint8_t count)
{
    switch (res) {
        case MOTEUS_RES_INT8:  return MOTEUS_OP_READ_INT8(count);
        case MOTEUS_RES_INT16: return MOTEUS_OP_READ_INT16(count);
        case MOTEUS_RES_INT32: return MOTEUS_OP_READ_INT32(count);
        case MOTEUS_RES_FLOAT: return MOTEUS_OP_READ_FLOAT(count);
        default:               return MOTEUS_OP_NOP;
    }
}

/* ============================================================================
 * Frame Buffer Helper
 * ============================================================================ */

/**
 * @brief Buffer for building CAN frames
 */
typedef struct {
    uint8_t data[64];   /**< Frame data buffer */
    size_t pos;         /**< Current write position */
} moteus_frame_builder_t;

/**
 * @brief Initialize frame builder
 */
static inline void moteus_frame_init(moteus_frame_builder_t* fb)
{
    fb->pos = 0;
}

/**
 * @brief Add a byte to the frame
 */
static inline bool moteus_frame_add_byte(moteus_frame_builder_t* fb, uint8_t byte)
{
    if (fb->pos >= 64) return false;
    fb->data[fb->pos++] = byte;
    return true;
}

/**
 * @brief Add a register address with varint encoding
 *
 * Moteus uses varint encoding for register addresses:
 * - For reg < 0x80: single byte
 * - For reg >= 0x80: two bytes (0x80 | low7, high7)
 */
static inline bool moteus_frame_add_register(moteus_frame_builder_t* fb, uint16_t reg)
{
    if (reg < 0x80) {
        return moteus_frame_add_byte(fb, (uint8_t)reg);
    } else {
        if (fb->pos + 2 > 64) return false;
        fb->data[fb->pos++] = 0x80 | (reg & 0x7F);
        fb->data[fb->pos++] = (reg >> 7) & 0x7F;
        return true;
    }
}

/**
 * @brief Add int8 value to frame
 */
static inline bool moteus_frame_add_int8(moteus_frame_builder_t* fb, int8_t value)
{
    return moteus_frame_add_byte(fb, (uint8_t)value);
}

/**
 * @brief Add int16 value to frame (little-endian)
 */
static inline bool moteus_frame_add_int16(moteus_frame_builder_t* fb, int16_t value)
{
    if (fb->pos + 2 > 64) return false;
    fb->data[fb->pos++] = value & 0xFF;
    fb->data[fb->pos++] = (value >> 8) & 0xFF;
    return true;
}

/**
 * @brief Add int32 value to frame (little-endian)
 */
static inline bool moteus_frame_add_int32(moteus_frame_builder_t* fb, int32_t value)
{
    if (fb->pos + 4 > 64) return false;
    fb->data[fb->pos++] = value & 0xFF;
    fb->data[fb->pos++] = (value >> 8) & 0xFF;
    fb->data[fb->pos++] = (value >> 16) & 0xFF;
    fb->data[fb->pos++] = (value >> 24) & 0xFF;
    return true;
}

/**
 * @brief Add float value to frame (IEEE 754, little-endian)
 */
static inline bool moteus_frame_add_float(moteus_frame_builder_t* fb, float value)
{
    if (fb->pos + 4 > 64) return false;
    union { float f; uint32_t u; } conv;
    conv.f = value;
    fb->data[fb->pos++] = conv.u & 0xFF;
    fb->data[fb->pos++] = (conv.u >> 8) & 0xFF;
    fb->data[fb->pos++] = (conv.u >> 16) & 0xFF;
    fb->data[fb->pos++] = (conv.u >> 24) & 0xFF;
    return true;
}

/* ============================================================================
 * Frame Building Functions - Basic Commands
 * ============================================================================ */

/**
 * @brief Build a stop command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_stop_frame(moteus_can_frame_t* frame,
                            uint8_t motor_id,
                            const moteus_query_resolution_t* query);

/**
 * @brief Build a brake mode frame (resists movement, allows backdrive)
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_brake_frame(moteus_can_frame_t* frame,
                             uint8_t motor_id,
                             const moteus_query_resolution_t* query);

/**
 * @brief Build a query-only frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param query Query resolution configuration
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_query_frame(moteus_can_frame_t* frame,
                             uint8_t motor_id,
                             const moteus_query_resolution_t* query);

/* ============================================================================
 * Frame Building Functions - Motion Commands
 * ============================================================================ */

/**
 * @brief Build a position command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param cmd Position command parameters
 * @param cmd_res Command resolution (NULL for defaults)
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_position_frame(moteus_can_frame_t* frame,
                                uint8_t motor_id,
                                const moteus_position_cmd_t* cmd,
                                const moteus_position_resolution_t* cmd_res,
                                const moteus_query_resolution_t* query);

/**
 * @brief Build a torque command frame (position mode with kp=0, kd=0)
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param torque Torque in Nm
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_torque_frame(moteus_can_frame_t* frame,
                              uint8_t motor_id,
                              float torque,
                              const moteus_query_resolution_t* query);

/**
 * @brief Build a stay-within command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param cmd Stay-within command parameters
 * @param cmd_res Command resolution (NULL for defaults)
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_stay_within_frame(moteus_can_frame_t* frame,
                                   uint8_t motor_id,
                                   const moteus_stay_within_cmd_t* cmd,
                                   const moteus_stay_within_resolution_t* cmd_res,
                                   const moteus_query_resolution_t* query);

/**
 * @brief Build a VFOC command frame (voltage field-oriented control)
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param cmd VFOC command parameters
 * @param cmd_res Command resolution (NULL for defaults)
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_vfoc_frame(moteus_can_frame_t* frame,
                            uint8_t motor_id,
                            const moteus_vfoc_cmd_t* cmd,
                            const moteus_vfoc_resolution_t* cmd_res,
                            const moteus_query_resolution_t* query);

/* ============================================================================
 * Frame Building Functions - Calibration/Rezero
 * ============================================================================ */

/**
 * @brief Build a rezero (set output nearest) command frame
 *
 * Sets the output position to the nearest value to the specified position.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param position Position value in revolutions
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_rezero_frame(moteus_can_frame_t* frame,
                              uint8_t motor_id,
                              float position,
                              const moteus_query_resolution_t* query);

/**
 * @brief Build a set output exact command frame
 *
 * Sets the output position to the exact specified value.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param position Position value in revolutions
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_set_output_exact_frame(moteus_can_frame_t* frame,
                                        uint8_t motor_id,
                                        float position,
                                        const moteus_query_resolution_t* query);

/**
 * @brief Build a require reindex command frame
 *
 * Forces the encoder to reindex on next motion.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_require_reindex_frame(moteus_can_frame_t* frame,
                                       uint8_t motor_id,
                                       const moteus_query_resolution_t* query);

/**
 * @brief Build a recapture position/velocity command frame
 *
 * Resynchronizes the position feedback.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_recapture_frame(moteus_can_frame_t* frame,
                                 uint8_t motor_id,
                                 const moteus_query_resolution_t* query);

/* ============================================================================
 * Frame Building Functions - GPIO
 * ============================================================================ */

/**
 * @brief Build a GPIO write command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param cmd GPIO command (aux1/aux2 output bits)
 * @param query Query resolution (NULL for no query)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_gpio_write_frame(moteus_can_frame_t* frame,
                                  uint8_t motor_id,
                                  const moteus_gpio_cmd_t* cmd,
                                  const moteus_query_resolution_t* query);

/**
 * @brief Build a GPIO read command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_gpio_read_frame(moteus_can_frame_t* frame,
                                 uint8_t motor_id);

/**
 * @brief Build an analog read command frame
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param read_aux1 True to read AUX1 analog inputs (5 channels)
 * @param read_aux2 True to read AUX2 analog inputs (5 channels)
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_analog_read_frame(moteus_can_frame_t* frame,
                                   uint8_t motor_id,
                                   bool read_aux1,
                                   bool read_aux2);

/* ============================================================================
 * Frame Building Functions - Diagnostics
 * ============================================================================ */

/**
 * @brief Build a diagnostic write frame
 *
 * Used to send configuration commands to the motor controller.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param channel Diagnostic channel (1=config, 2=telemetry)
 * @param data Data to write
 * @param len Data length
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_diagnostic_write_frame(moteus_can_frame_t* frame,
                                        uint8_t motor_id,
                                        uint8_t channel,
                                        const uint8_t* data,
                                        uint8_t len);

/**
 * @brief Build a diagnostic read (poll) frame
 *
 * Used to request diagnostic response from the motor controller.
 *
 * @param frame Output frame structure
 * @param motor_id Target motor ID
 * @param channel Diagnostic channel
 * @return Number of bytes in frame, or negative error code
 */
int moteus_build_diagnostic_read_frame(moteus_can_frame_t* frame,
                                       uint8_t motor_id,
                                       uint8_t channel);

/* ============================================================================
 * Frame Parsing Functions
 * ============================================================================ */

/**
 * @brief Parse a response frame from Moteus
 *
 * @param data Frame data
 * @param len Frame length
 * @param result Output result structure
 * @return 0 on success, negative error code on failure
 */
int moteus_parse_response(const uint8_t* data,
                          size_t len,
                          moteus_result_t* result);

/**
 * @brief Parse a diagnostic response frame
 *
 * @param data Frame data
 * @param len Frame length
 * @param result Output diagnostic result
 * @return 0 on success, negative error code on failure
 */
int moteus_parse_diagnostic_response(const uint8_t* data,
                                     size_t len,
                                     moteus_diagnostic_result_t* result);

/**
 * @brief Parse a GPIO read response
 *
 * @param data Frame data
 * @param len Frame length
 * @param result Output GPIO result
 * @return 0 on success, negative error code on failure
 */
int moteus_parse_gpio_response(const uint8_t* data,
                               size_t len,
                               moteus_gpio_result_t* result);

/**
 * @brief Parse an analog read response
 *
 * @param data Frame data
 * @param len Frame length
 * @param result Output analog result (aux1[5], aux2[5])
 * @return 0 on success, negative error code on failure
 */
int moteus_parse_analog_response(const uint8_t* data,
                                 size_t len,
                                 moteus_analog_result_t* result);

/* ============================================================================
 * Value Encoding/Decoding
 * ============================================================================ */

/**
 * @brief Encode a float to a scaled integer
 *
 * @param value Float value to encode
 * @param scale Scale factor (value is divided by this)
 * @param res Target resolution
 * @param out Output buffer (must be large enough for resolution)
 * @return Number of bytes written
 */
int moteus_encode_value(float value, float scale, moteus_resolution_t res, uint8_t* out);

/**
 * @brief Decode a scaled integer to float
 *
 * @param data Input data (little-endian)
 * @param scale Scale factor (result is multiplied by this)
 * @param res Source resolution
 * @return Decoded float value
 */
float moteus_decode_value(const uint8_t* data, float scale, moteus_resolution_t res);

#ifdef __cplusplus
}
#endif

#endif /* MOTEUS_PROTOCOL_H */
