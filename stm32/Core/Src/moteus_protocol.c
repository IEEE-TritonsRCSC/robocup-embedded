/**
 * @file moteus_protocol.c
 * @brief Moteus CAN protocol encoding/decoding implementation
 */

#include "moteus_protocol.h"
#include "moteus.h"  /* For MOTEUS_ENABLE_BRS */
#include <string.h>
#include <math.h>

/* ============================================================================
 * Value Encoding/Decoding
 * ============================================================================ */

int moteus_encode_value(float value, float scale, moteus_resolution_t res, uint8_t* out)
{
    if (res == MOTEUS_RES_IGNORE) {
        return 0;
    }

    if (res == MOTEUS_RES_FLOAT) {
        union { float f; uint32_t u; } conv;
        conv.f = value;
        out[0] = conv.u & 0xFF;
        out[1] = (conv.u >> 8) & 0xFF;
        out[2] = (conv.u >> 16) & 0xFF;
        out[3] = (conv.u >> 24) & 0xFF;
        return 4;
    }

    /* Scale and clamp */
    float scaled = value / scale;

    switch (res) {
        case MOTEUS_RES_INT8: {
            int32_t clamped = (int32_t)roundf(scaled);
            if (clamped > 127) clamped = 127;
            if (clamped < -128) clamped = -128;
            out[0] = (uint8_t)(int8_t)clamped;
            return 1;
        }
        case MOTEUS_RES_INT16: {
            int32_t clamped = (int32_t)roundf(scaled);
            if (clamped > 32767) clamped = 32767;
            if (clamped < -32768) clamped = -32768;
            out[0] = clamped & 0xFF;
            out[1] = (clamped >> 8) & 0xFF;
            return 2;
        }
        case MOTEUS_RES_INT32: {
            int32_t ival = (int32_t)roundf(scaled);
            out[0] = ival & 0xFF;
            out[1] = (ival >> 8) & 0xFF;
            out[2] = (ival >> 16) & 0xFF;
            out[3] = (ival >> 24) & 0xFF;
            return 4;
        }
        default:
            return 0;
    }
}

float moteus_decode_value(const uint8_t* data, float scale, moteus_resolution_t res)
{
    switch (res) {
        case MOTEUS_RES_INT8: {
            int8_t val = (int8_t)data[0];
            return (float)val * scale;
        }
        case MOTEUS_RES_INT16: {
            int16_t val = (int16_t)(data[0] | (data[1] << 8));
            return (float)val * scale;
        }
        case MOTEUS_RES_INT32: {
            int32_t val = (int32_t)(data[0] | (data[1] << 8) |
                                    (data[2] << 16) | (data[3] << 24));
            return (float)val * scale;
        }
        case MOTEUS_RES_FLOAT: {
            union { float f; uint32_t u; } conv;
            conv.u = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            return conv.f;
        }
        default:
            return NAN;
    }
}

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

/**
 * @brief Get scale factor for a query register
 */
static float get_query_scale(uint8_t reg, moteus_resolution_t res)
{
    if (res == MOTEUS_RES_FLOAT) return 1.0f;

    switch (reg) {
        case MOTEUS_REG_Q_MODE:
        case MOTEUS_REG_Q_FAULT:
        case MOTEUS_REG_Q_REZERO_STATE:
        case MOTEUS_REG_Q_TRAJECTORY_COMPLETE:
            return 1.0f;

        case MOTEUS_REG_Q_POSITION:
        case MOTEUS_REG_Q_ABS_POSITION:
        case MOTEUS_REG_Q_ENCODER_0_POSITION:
        case MOTEUS_REG_Q_ENCODER_1_POSITION:
        case MOTEUS_REG_Q_ENCODER_2_POSITION:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_POSITION_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_POSITION_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_POSITION_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_VELOCITY:
        case MOTEUS_REG_Q_ENCODER_0_VELOCITY:
        case MOTEUS_REG_Q_ENCODER_1_VELOCITY:
        case MOTEUS_REG_Q_ENCODER_2_VELOCITY:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_VELOCITY_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_VELOCITY_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_VELOCITY_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_TORQUE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_TORQUE_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_TORQUE_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_TORQUE_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_Q_CURRENT:
        case MOTEUS_REG_Q_D_CURRENT:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_CURRENT_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_CURRENT_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_CURRENT_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_VOLTAGE:
        case MOTEUS_REG_Q_VOLTAGE_PHASE_A:
        case MOTEUS_REG_Q_VOLTAGE_PHASE_B:
        case MOTEUS_REG_Q_VOLTAGE_PHASE_C:
        case MOTEUS_REG_Q_VFOC_VOLTAGE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_VOLTAGE_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_VOLTAGE_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_VOLTAGE_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_TEMPERATURE:
        case MOTEUS_REG_Q_MOTOR_TEMPERATURE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_TEMPERATURE_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_TEMPERATURE_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_TEMPERATURE_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_POWER:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_POWER_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_POWER_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_POWER_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_VFOC_THETA:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_THETA_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_THETA_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_THETA_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_Q_PWM_PHASE_A:
        case MOTEUS_REG_Q_PWM_PHASE_B:
        case MOTEUS_REG_Q_PWM_PHASE_C:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_PWM_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_PWM_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_PWM_INT32;
                default: return 1.0f;
            }

        default:
            return 1.0f;
    }
}

/**
 * @brief Get scale factor for a command register
 */
static float get_command_scale(uint16_t reg, moteus_resolution_t res)
{
    if (res == MOTEUS_RES_FLOAT) return 1.0f;

    switch (reg) {
        case MOTEUS_REG_MODE:
            return 1.0f;

        case MOTEUS_REG_POSITION:
        case MOTEUS_REG_COMMANDED_STOP_POSITION:
        case MOTEUS_REG_STAY_WITHIN_LOWER:
        case MOTEUS_REG_STAY_WITHIN_UPPER:
        case MOTEUS_REG_SET_OUTPUT_NEAREST:
        case MOTEUS_REG_SET_OUTPUT_EXACT:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_POSITION_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_POSITION_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_POSITION_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_VELOCITY:
        case MOTEUS_REG_VELOCITY_LIMIT:
        case MOTEUS_REG_STAY_WITHIN_VELOCITY_LIMIT:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_VELOCITY_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_VELOCITY_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_VELOCITY_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_FEEDFORWARD_TORQUE:
        case MOTEUS_REG_MAX_TORQUE:
        case MOTEUS_REG_STAY_WITHIN_FF_TORQUE:
        case MOTEUS_REG_STAY_WITHIN_MAX_TORQUE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_TORQUE_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_TORQUE_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_TORQUE_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_KP_SCALE:
        case MOTEUS_REG_KD_SCALE:
        case MOTEUS_REG_STAY_WITHIN_KP_SCALE:
        case MOTEUS_REG_STAY_WITHIN_KD_SCALE:
        case MOTEUS_REG_ILIMIT_SCALE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_KP_KD_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_KP_KD_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_KP_KD_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_WATCHDOG_TIMEOUT:
        case MOTEUS_REG_STAY_WITHIN_TIMEOUT:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_TIME_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_TIME_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_TIME_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_ACCEL_LIMIT:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_ACCEL_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_ACCEL_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_ACCEL_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_VFOC_THETA:
        case MOTEUS_REG_VFOC_THETA_RATE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_THETA_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_THETA_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_THETA_INT32;
                default: return 1.0f;
            }

        case MOTEUS_REG_VFOC_VOLTAGE:
            switch (res) {
                case MOTEUS_RES_INT8:  return MOTEUS_SCALE_VOLTAGE_INT8;
                case MOTEUS_RES_INT16: return MOTEUS_SCALE_VOLTAGE_INT16;
                case MOTEUS_RES_INT32: return MOTEUS_SCALE_VOLTAGE_INT32;
                default: return 1.0f;
            }

        default:
            return 1.0f;
    }
}

/**
 * @brief Add query request to frame
 */
static int add_query_request(moteus_frame_builder_t* fb, const moteus_query_resolution_t* query)
{
    if (query == NULL) return 0;

    /* Check if we can do a contiguous read of registers 0x00-0x03 (mode, pos, vel, torque) */
    bool contiguous_0_3 = (query->mode != MOTEUS_RES_IGNORE &&
                           query->position != MOTEUS_RES_IGNORE &&
                           query->velocity != MOTEUS_RES_IGNORE &&
                           query->torque != MOTEUS_RES_IGNORE &&
                           query->mode == query->position &&
                           query->position == query->velocity &&
                           query->velocity == query->torque);

    if (contiguous_0_3) {
        /* Read 4 contiguous registers */
        moteus_frame_add_byte(fb, moteus_read_opcode(query->mode, 4));
        moteus_frame_add_byte(fb, MOTEUS_REG_Q_MODE);
    } else {
        /* Read individually */
        if (query->mode != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->mode, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_MODE);
        }
        if (query->position != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->position, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_POSITION);
        }
        if (query->velocity != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->velocity, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_VELOCITY);
        }
        if (query->torque != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->torque, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_TORQUE);
        }
    }

    /* Q/D current */
    if (query->q_current != MOTEUS_RES_IGNORE && query->d_current != MOTEUS_RES_IGNORE &&
        query->q_current == query->d_current) {
        moteus_frame_add_byte(fb, moteus_read_opcode(query->q_current, 2));
        moteus_frame_add_byte(fb, MOTEUS_REG_Q_Q_CURRENT);
    } else {
        if (query->q_current != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->q_current, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_Q_CURRENT);
        }
        if (query->d_current != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->d_current, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_D_CURRENT);
        }
    }

    /* Voltage, temperature, fault (0x0D, 0x0E, 0x0F) */
    bool contiguous_d_f = (query->voltage != MOTEUS_RES_IGNORE &&
                           query->temperature != MOTEUS_RES_IGNORE &&
                           query->fault != MOTEUS_RES_IGNORE &&
                           query->voltage == query->temperature &&
                           query->temperature == query->fault);

    if (contiguous_d_f) {
        moteus_frame_add_byte(fb, moteus_read_opcode(query->voltage, 3));
        moteus_frame_add_byte(fb, MOTEUS_REG_Q_VOLTAGE);
    } else {
        if (query->voltage != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->voltage, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_VOLTAGE);
        }
        if (query->temperature != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->temperature, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_TEMPERATURE);
        }
        if (query->fault != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(fb, moteus_read_opcode(query->fault, 1));
            moteus_frame_add_byte(fb, MOTEUS_REG_Q_FAULT);
        }
    }

    return 0;
}

/**
 * @brief Helper to finalize frame
 */
static void finalize_frame(moteus_can_frame_t* frame, uint8_t motor_id,
                           const moteus_frame_builder_t* fb, bool request_reply)
{
    frame->id = moteus_make_can_id(motor_id, MOTEUS_SOURCE_ID, request_reply);
    memcpy(frame->data, fb->data, fb->pos);
    frame->len = fb->pos;
    frame->is_fd = true;
#if MOTEUS_ENABLE_BRS
    frame->brs = true;
#else
    frame->brs = false;
#endif
}

/* ============================================================================
 * Frame Building Functions - Basic Commands
 * ============================================================================ */

int moteus_build_stop_frame(moteus_can_frame_t* frame,
                            uint8_t motor_id,
                            const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write mode = stopped */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_byte(&fb, MOTEUS_REG_MODE);
    moteus_frame_add_byte(&fb, MOTEUS_MODE_STOPPED);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_brake_frame(moteus_can_frame_t* frame,
                             uint8_t motor_id,
                             const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write mode = brake */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_byte(&fb, MOTEUS_REG_MODE);
    moteus_frame_add_byte(&fb, MOTEUS_MODE_BRAKE);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_query_frame(moteus_can_frame_t* frame,
                             uint8_t motor_id,
                             const moteus_query_resolution_t* query)
{
    if (query == NULL) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, true);

    return fb.pos;
}

/* ============================================================================
 * Frame Building Functions - Motion Commands
 * ============================================================================ */

int moteus_build_position_frame(moteus_can_frame_t* frame,
                                uint8_t motor_id,
                                const moteus_position_cmd_t* cmd,
                                const moteus_position_resolution_t* cmd_res,
                                const moteus_query_resolution_t* query)
{
    if (cmd == NULL) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_position_resolution_t default_res = MOTEUS_POSITION_RESOLUTION_DEFAULT;
    if (cmd_res == NULL) {
        cmd_res = &default_res;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write mode = position */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_byte(&fb, MOTEUS_REG_MODE);
    moteus_frame_add_byte(&fb, MOTEUS_MODE_POSITION);

    /* Build array of fields to write */
    struct {
        uint8_t reg;
        float value;
        moteus_resolution_t res;
    } fields[6];
    int field_count = 0;

    if (cmd_res->position != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_POSITION;
        fields[field_count].value = cmd->position;
        fields[field_count].res = cmd_res->position;
        field_count++;
    }
    if (cmd_res->velocity != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_VELOCITY;
        fields[field_count].value = isnan(cmd->velocity) ? 0.0f : cmd->velocity;
        fields[field_count].res = cmd_res->velocity;
        field_count++;
    }
    if (cmd_res->feedforward_torque != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_FEEDFORWARD_TORQUE;
        fields[field_count].value = isnan(cmd->feedforward_torque) ? 0.0f : cmd->feedforward_torque;
        fields[field_count].res = cmd_res->feedforward_torque;
        field_count++;
    }
    if (cmd_res->kp_scale != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_KP_SCALE;
        fields[field_count].value = isnan(cmd->kp_scale) ? 1.0f : cmd->kp_scale;
        fields[field_count].res = cmd_res->kp_scale;
        field_count++;
    }
    if (cmd_res->kd_scale != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_KD_SCALE;
        fields[field_count].value = isnan(cmd->kd_scale) ? 1.0f : cmd->kd_scale;
        fields[field_count].res = cmd_res->kd_scale;
        field_count++;
    }
    if (cmd_res->max_torque != MOTEUS_RES_IGNORE) {
        fields[field_count].reg = MOTEUS_REG_MAX_TORQUE;
        fields[field_count].value = isnan(cmd->max_torque) ? 100.0f : cmd->max_torque;
        fields[field_count].res = cmd_res->max_torque;
        field_count++;
    }

    /* Write fields, grouping consecutive fields with same resolution */
    uint8_t buf[4];
    float scale;
    int i = 0;
    while (i < field_count) {
        /* Count consecutive fields with same resolution */
        moteus_resolution_t res = fields[i].res;
        int count = 1;
        while (i + count < field_count &&
               fields[i + count].res == res &&
               fields[i + count].reg == fields[i].reg + count &&
               count < 4) {
            count++;
        }

        /* Emit write opcode */
        if (count <= 3) {
            moteus_frame_add_byte(&fb, moteus_write_opcode(res, count));
        } else {
            moteus_frame_add_byte(&fb, moteus_write_opcode(res, 0));
            moteus_frame_add_byte(&fb, count);
        }
        moteus_frame_add_byte(&fb, fields[i].reg);

        /* Emit values */
        for (int j = 0; j < count; j++) {
            scale = get_command_scale(fields[i + j].reg, res);
            moteus_encode_value(fields[i + j].value, scale, res, buf);
            for (size_t k = 0; k < moteus_resolution_size(res); k++) {
                moteus_frame_add_byte(&fb, buf[k]);
            }
        }
        i += count;
    }

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_torque_frame(moteus_can_frame_t* frame,
                              uint8_t motor_id,
                              float torque,
                              const moteus_query_resolution_t* query)
{
    moteus_position_cmd_t cmd = MOTEUS_POSITION_CMD_DEFAULT;
    cmd.position = NAN;
    cmd.velocity = 0.0f;
    cmd.feedforward_torque = torque;
    cmd.kp_scale = 0.0f;
    cmd.kd_scale = 0.0f;
    cmd.max_torque = fabsf(torque) + 0.1f;

    return moteus_build_position_frame(frame, motor_id, &cmd, NULL, query);
}

int moteus_build_stay_within_frame(moteus_can_frame_t* frame,
                                   uint8_t motor_id,
                                   const moteus_stay_within_cmd_t* cmd,
                                   const moteus_stay_within_resolution_t* cmd_res,
                                   const moteus_query_resolution_t* query)
{
    if (cmd == NULL) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_stay_within_resolution_t default_res = MOTEUS_STAY_WITHIN_RESOLUTION_DEFAULT;
    if (cmd_res == NULL) {
        cmd_res = &default_res;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write mode = stay_within */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_byte(&fb, MOTEUS_REG_MODE);
    moteus_frame_add_byte(&fb, MOTEUS_MODE_STAY_WITHIN);

    /* Write stay-within registers starting at 0x40 */
    uint8_t buf[4];
    float scale;
    moteus_resolution_t res = cmd_res->lower_bound;

    /* Count contiguous registers */
    int contiguous_count = 0;
    if (cmd_res->lower_bound != MOTEUS_RES_IGNORE) {
        contiguous_count = 1;
        if (cmd_res->upper_bound == res) {
            contiguous_count = 2;
            if (cmd_res->feedforward_torque == res) {
                contiguous_count = 3;
                if (cmd_res->kp_scale == res) {
                    contiguous_count = 4;
                    if (cmd_res->kd_scale == res) {
                        contiguous_count = 5;
                        if (cmd_res->max_torque == res) {
                            contiguous_count = 6;
                        }
                    }
                }
            }
        }
    }

    if (contiguous_count > 0) {
        moteus_frame_add_byte(&fb, moteus_write_opcode(res, contiguous_count));
        moteus_frame_add_byte(&fb, MOTEUS_REG_STAY_WITHIN_LOWER);

        /* Lower bound */
        scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_LOWER, res);
        moteus_encode_value(cmd->lower_bound, scale, res, buf);
        for (size_t i = 0; i < moteus_resolution_size(res); i++) {
            moteus_frame_add_byte(&fb, buf[i]);
        }

        if (contiguous_count >= 2) {
            scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_UPPER, res);
            moteus_encode_value(cmd->upper_bound, scale, res, buf);
            for (size_t i = 0; i < moteus_resolution_size(res); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }

        if (contiguous_count >= 3) {
            float ff = isnan(cmd->feedforward_torque) ? 0.0f : cmd->feedforward_torque;
            scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_FF_TORQUE, res);
            moteus_encode_value(ff, scale, res, buf);
            for (size_t i = 0; i < moteus_resolution_size(res); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }

        if (contiguous_count >= 4) {
            float kp = isnan(cmd->kp_scale) ? 1.0f : cmd->kp_scale;
            scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_KP_SCALE, res);
            moteus_encode_value(kp, scale, res, buf);
            for (size_t i = 0; i < moteus_resolution_size(res); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }

        if (contiguous_count >= 5) {
            float kd = isnan(cmd->kd_scale) ? 1.0f : cmd->kd_scale;
            scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_KD_SCALE, res);
            moteus_encode_value(kd, scale, res, buf);
            for (size_t i = 0; i < moteus_resolution_size(res); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }

        if (contiguous_count >= 6) {
            float max_t = isnan(cmd->max_torque) ? 100.0f : cmd->max_torque;
            scale = get_command_scale(MOTEUS_REG_STAY_WITHIN_MAX_TORQUE, res);
            moteus_encode_value(max_t, scale, res, buf);
            for (size_t i = 0; i < moteus_resolution_size(res); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }
    }

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_vfoc_frame(moteus_can_frame_t* frame,
                            uint8_t motor_id,
                            const moteus_vfoc_cmd_t* cmd,
                            const moteus_vfoc_resolution_t* cmd_res,
                            const moteus_query_resolution_t* query)
{
    if (cmd == NULL) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_vfoc_resolution_t default_res = MOTEUS_VFOC_RESOLUTION_DEFAULT;
    if (cmd_res == NULL) {
        cmd_res = &default_res;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write mode = voltage_foc */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_byte(&fb, MOTEUS_REG_MODE);
    moteus_frame_add_byte(&fb, MOTEUS_MODE_VOLTAGE_FOC);

    /* Write VFOC registers (0x18, 0x19, optionally 0x1E) */
    uint8_t buf[4];
    float scale;
    moteus_resolution_t res = cmd_res->theta;

    if (cmd_res->theta != MOTEUS_RES_IGNORE && cmd_res->voltage == res) {
        /* Write theta and voltage contiguously */
        moteus_frame_add_byte(&fb, moteus_write_opcode(res, 2));
        moteus_frame_add_byte(&fb, MOTEUS_REG_VFOC_THETA);

        scale = get_command_scale(MOTEUS_REG_VFOC_THETA, res);
        moteus_encode_value(cmd->theta, scale, res, buf);
        for (size_t i = 0; i < moteus_resolution_size(res); i++) {
            moteus_frame_add_byte(&fb, buf[i]);
        }

        scale = get_command_scale(MOTEUS_REG_VFOC_VOLTAGE, res);
        moteus_encode_value(cmd->voltage, scale, res, buf);
        for (size_t i = 0; i < moteus_resolution_size(res); i++) {
            moteus_frame_add_byte(&fb, buf[i]);
        }
    } else {
        /* Write individually */
        if (cmd_res->theta != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(&fb, moteus_write_opcode(cmd_res->theta, 1));
            moteus_frame_add_byte(&fb, MOTEUS_REG_VFOC_THETA);
            scale = get_command_scale(MOTEUS_REG_VFOC_THETA, cmd_res->theta);
            moteus_encode_value(cmd->theta, scale, cmd_res->theta, buf);
            for (size_t i = 0; i < moteus_resolution_size(cmd_res->theta); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }
        if (cmd_res->voltage != MOTEUS_RES_IGNORE) {
            moteus_frame_add_byte(&fb, moteus_write_opcode(cmd_res->voltage, 1));
            moteus_frame_add_byte(&fb, MOTEUS_REG_VFOC_VOLTAGE);
            scale = get_command_scale(MOTEUS_REG_VFOC_VOLTAGE, cmd_res->voltage);
            moteus_encode_value(cmd->voltage, scale, cmd_res->voltage, buf);
            for (size_t i = 0; i < moteus_resolution_size(cmd_res->voltage); i++) {
                moteus_frame_add_byte(&fb, buf[i]);
            }
        }
    }

    /* Theta rate (separate register 0x1E) */
    if (cmd_res->theta_rate != MOTEUS_RES_IGNORE && !isnan(cmd->theta_rate)) {
        moteus_frame_add_byte(&fb, moteus_write_opcode(cmd_res->theta_rate, 1));
        moteus_frame_add_byte(&fb, MOTEUS_REG_VFOC_THETA_RATE);
        scale = get_command_scale(MOTEUS_REG_VFOC_THETA_RATE, cmd_res->theta_rate);
        moteus_encode_value(cmd->theta_rate, scale, cmd_res->theta_rate, buf);
        for (size_t i = 0; i < moteus_resolution_size(cmd_res->theta_rate); i++) {
            moteus_frame_add_byte(&fb, buf[i]);
        }
    }

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

/* ============================================================================
 * Frame Building Functions - Calibration/Rezero
 * ============================================================================ */

int moteus_build_rezero_frame(moteus_can_frame_t* frame,
                              uint8_t motor_id,
                              float position,
                              const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write to SET_OUTPUT_NEAREST register (0x130) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_FLOAT(1));
    moteus_frame_add_register(&fb, MOTEUS_REG_SET_OUTPUT_NEAREST);
    moteus_frame_add_float(&fb, position);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_set_output_exact_frame(moteus_can_frame_t* frame,
                                        uint8_t motor_id,
                                        float position,
                                        const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write to SET_OUTPUT_EXACT register (0x131) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_FLOAT(1));
    moteus_frame_add_register(&fb, MOTEUS_REG_SET_OUTPUT_EXACT);
    moteus_frame_add_float(&fb, position);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_require_reindex_frame(moteus_can_frame_t* frame,
                                       uint8_t motor_id,
                                       const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write 1 to REQUIRE_REINDEX register (0x132) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_register(&fb, MOTEUS_REG_REQUIRE_REINDEX);
    moteus_frame_add_byte(&fb, 1);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_recapture_frame(moteus_can_frame_t* frame,
                                 uint8_t motor_id,
                                 const moteus_query_resolution_t* query)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write 1 to RECAPTURE register (0x133) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(1));
    moteus_frame_add_register(&fb, MOTEUS_REG_RECAPTURE_POSITION_VEL);
    moteus_frame_add_byte(&fb, 1);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

/* ============================================================================
 * Frame Building Functions - GPIO
 * ============================================================================ */

int moteus_build_gpio_write_frame(moteus_can_frame_t* frame,
                                  uint8_t motor_id,
                                  const moteus_gpio_cmd_t* cmd,
                                  const moteus_query_resolution_t* query)
{
    if (cmd == NULL) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Write AUX1 and AUX2 GPIO command registers (0x5C, 0x5D) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_WRITE_INT8(2));
    moteus_frame_add_byte(&fb, MOTEUS_REG_AUX1_GPIO_COMMAND);
    moteus_frame_add_byte(&fb, cmd->aux1);
    moteus_frame_add_byte(&fb, cmd->aux2);

    add_query_request(&fb, query);
    finalize_frame(frame, motor_id, &fb, query != NULL);

    return fb.pos;
}

int moteus_build_gpio_read_frame(moteus_can_frame_t* frame,
                                 uint8_t motor_id)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Read AUX1 and AUX2 GPIO status registers (0x5C, 0x5D) */
    moteus_frame_add_byte(&fb, MOTEUS_OP_READ_INT8(2));
    moteus_frame_add_byte(&fb, MOTEUS_REG_Q_AUX1_GPIO_STATUS);

    finalize_frame(frame, motor_id, &fb, true);

    return fb.pos;
}

int moteus_build_analog_read_frame(moteus_can_frame_t* frame,
                                   uint8_t motor_id,
                                   bool read_aux1,
                                   bool read_aux2)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Read AUX1 analog inputs (registers 0x60-0x64, 5 floats) */
    if (read_aux1) {
        moteus_frame_add_byte(&fb, MOTEUS_OP_READ_FLOAT(0));  /* 0 = use varint count */
        moteus_frame_add_byte(&fb, 5);  /* 5 registers */
        moteus_frame_add_byte(&fb, MOTEUS_REG_Q_AUX1_ANALOG_IN1);
    }

    /* Read AUX2 analog inputs (registers 0x68-0x6C, 5 floats) */
    if (read_aux2) {
        moteus_frame_add_byte(&fb, MOTEUS_OP_READ_FLOAT(0));
        moteus_frame_add_byte(&fb, 5);
        moteus_frame_add_byte(&fb, MOTEUS_REG_Q_AUX2_ANALOG_IN1);
    }

    finalize_frame(frame, motor_id, &fb, true);

    return fb.pos;
}

/* ============================================================================
 * Frame Building Functions - Diagnostics
 * ============================================================================ */

int moteus_build_diagnostic_write_frame(moteus_can_frame_t* frame,
                                        uint8_t motor_id,
                                        uint8_t channel,
                                        const uint8_t* data,
                                        uint8_t len)
{
    if (data == NULL || len == 0 || len > 60) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Client to server diagnostic write */
    moteus_frame_add_byte(&fb, MOTEUS_OP_CLIENT_TO_SERVER);
    moteus_frame_add_byte(&fb, channel);
    moteus_frame_add_byte(&fb, len);

    for (uint8_t i = 0; i < len; i++) {
        moteus_frame_add_byte(&fb, data[i]);
    }

    finalize_frame(frame, motor_id, &fb, true);

    return fb.pos;
}

int moteus_build_diagnostic_read_frame(moteus_can_frame_t* frame,
                                       uint8_t motor_id,
                                       uint8_t channel)
{
    moteus_frame_builder_t fb;
    moteus_frame_init(&fb);

    /* Client poll for diagnostic response */
    moteus_frame_add_byte(&fb, MOTEUS_OP_CLIENT_POLL);
    moteus_frame_add_byte(&fb, channel);
    moteus_frame_add_byte(&fb, 48);  /* max_length - request up to 48 bytes */

    finalize_frame(frame, motor_id, &fb, true);

    return fb.pos;
}

/* ============================================================================
 * Frame Parsing Functions
 * ============================================================================ */

int moteus_parse_response(const uint8_t* data,
                          size_t len,
                          moteus_result_t* result)
{
    if (data == NULL || result == NULL || len == 0) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    /* Initialize result with NaN/defaults */
    result->mode = 0;
    result->fault = 0;
    result->position = NAN;
    result->velocity = NAN;
    result->torque = NAN;
    result->q_current = NAN;
    result->d_current = NAN;
    result->voltage = NAN;
    result->temperature = NAN;
    result->motor_temperature = NAN;
    result->power = NAN;
    result->abs_position = NAN;
    result->rezero_state = 0;
    result->trajectory_complete = false;

    size_t pos = 0;

    while (pos < len) {
        uint8_t opcode = data[pos++];

        /* NOP */
        if (opcode == MOTEUS_OP_NOP) {
            continue;
        }

        /* Error opcodes */
        if (opcode == MOTEUS_OP_WRITE_ERROR || opcode == MOTEUS_OP_READ_ERROR) {
            return MOTEUS_ERR_INVALID_FRAME;
        }

        /* Check if this is a reply opcode (0x20-0x2F) */
        if ((opcode & 0xF0) != 0x20) {
            continue;
        }

        /* Extract type and count from opcode
         * Format: 0b0010TTCC where TT=type, CC=count
         * Type: 00=int8, 01=int16, 10=int32, 11=float
         * Count: 0-3 (0 typically means 1)
         */
        uint8_t type_bits = (opcode >> 2) & 0x03;
        uint8_t count = opcode & 0x03;
        if (count == 0) count = 1;

        moteus_resolution_t res;
        switch (type_bits) {
            case 0: res = MOTEUS_RES_INT8; break;
            case 1: res = MOTEUS_RES_INT16; break;
            case 2: res = MOTEUS_RES_INT32; break;
            case 3: res = MOTEUS_RES_FLOAT; break;
            default: res = MOTEUS_RES_INT8; break;
        }

        if (pos >= len) break;

        /* Get starting register */
        uint8_t start_reg = data[pos++];
        size_t value_size = moteus_resolution_size(res);

        /* Read values */
        for (uint8_t i = 0; i < count && pos + value_size <= len; i++) {
            uint8_t reg = start_reg + i;
            float scale = get_query_scale(reg, res);
            float value = moteus_decode_value(&data[pos], scale, res);
            pos += value_size;

            /* Store in result */
            switch (reg) {
                case MOTEUS_REG_Q_MODE:
                    result->mode = (uint8_t)value;
                    break;
                case MOTEUS_REG_Q_POSITION:
                    result->position = value;
                    break;
                case MOTEUS_REG_Q_VELOCITY:
                    result->velocity = value;
                    break;
                case MOTEUS_REG_Q_TORQUE:
                    result->torque = value;
                    break;
                case MOTEUS_REG_Q_Q_CURRENT:
                    result->q_current = value;
                    break;
                case MOTEUS_REG_Q_D_CURRENT:
                    result->d_current = value;
                    break;
                case MOTEUS_REG_Q_ABS_POSITION:
                    result->abs_position = value;
                    break;
                case MOTEUS_REG_Q_POWER:
                    result->power = value;
                    break;
                case MOTEUS_REG_Q_MOTOR_TEMPERATURE:
                    result->motor_temperature = value;
                    break;
                case MOTEUS_REG_Q_TRAJECTORY_COMPLETE:
                    result->trajectory_complete = (value != 0.0f);
                    break;
                case MOTEUS_REG_Q_REZERO_STATE:
                    result->rezero_state = (uint8_t)value;
                    break;
                case MOTEUS_REG_Q_VOLTAGE:
                    result->voltage = value;
                    break;
                case MOTEUS_REG_Q_TEMPERATURE:
                    result->temperature = value;
                    break;
                case MOTEUS_REG_Q_FAULT:
                    result->fault = (uint8_t)value;
                    break;
                default:
                    break;
            }
        }
    }

    return MOTEUS_OK;
}

int moteus_parse_diagnostic_response(const uint8_t* data,
                                     size_t len,
                                     moteus_diagnostic_result_t* result)
{
    if (data == NULL || result == NULL || len < 3) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    /* Check for server to client opcode */
    if (data[0] != MOTEUS_OP_SERVER_TO_CLIENT) {
        return MOTEUS_ERR_INVALID_FRAME;
    }

    result->channel = data[1];
    result->len = data[2];

    if (result->len > 60 || result->len + 3 > len) {
        return MOTEUS_ERR_INVALID_FRAME;
    }

    memcpy(result->data, &data[3], result->len);

    return MOTEUS_OK;
}

int moteus_parse_gpio_response(const uint8_t* data,
                               size_t len,
                               moteus_gpio_result_t* result)
{
    if (data == NULL || result == NULL || len < 4) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    /* Look for reply opcode with 2 INT8 values */
    size_t pos = 0;
    while (pos < len) {
        uint8_t opcode = data[pos++];

        if ((opcode & 0xFC) == 0x20) {
            /* INT8 reply */
            uint8_t count = opcode & 0x03;
            if (count == 0) count = opcode & 0x0F;
            if (pos >= len) break;

            uint8_t start_reg = data[pos++];
            if (start_reg == MOTEUS_REG_Q_AUX1_GPIO_STATUS && count >= 2) {
                if (pos + 2 <= len) {
                    result->aux1 = data[pos++];
                    result->aux2 = data[pos++];
                    return MOTEUS_OK;
                }
            }
        }
    }

    return MOTEUS_ERR_INVALID_FRAME;
}

int moteus_parse_analog_response(const uint8_t* data,
                                 size_t len,
                                 moteus_analog_result_t* result)
{
    if (data == NULL || result == NULL || len < 3) {
        return MOTEUS_ERR_INVALID_PARAM;
    }

    /* Initialize to NaN */
    for (int i = 0; i < 5; i++) {
        result->aux1[i] = NAN;
        result->aux2[i] = NAN;
    }

    size_t pos = 0;
    while (pos < len) {
        uint8_t opcode = data[pos++];

        /* Check for float reply opcode (0x2C-0x2F) */
        if ((opcode & 0xFC) == 0x2C) {
            uint8_t count = opcode & 0x03;
            if (count == 0 && pos < len) {
                count = data[pos++];  /* Extended count */
            }
            if (pos >= len) break;

            uint8_t start_reg = data[pos++];

            /* Read float values */
            for (uint8_t i = 0; i < count && pos + 4 <= len; i++) {
                uint8_t reg = start_reg + i;
                float value = moteus_decode_value(&data[pos], 1.0f, MOTEUS_RES_FLOAT);
                pos += 4;

                /* Store based on register */
                if (reg >= MOTEUS_REG_Q_AUX1_ANALOG_IN1 && reg <= MOTEUS_REG_Q_AUX1_ANALOG_IN5) {
                    result->aux1[reg - MOTEUS_REG_Q_AUX1_ANALOG_IN1] = value;
                } else if (reg >= MOTEUS_REG_Q_AUX2_ANALOG_IN1 && reg <= MOTEUS_REG_Q_AUX2_ANALOG_IN5) {
                    result->aux2[reg - MOTEUS_REG_Q_AUX2_ANALOG_IN1] = value;
                }
            }
        }
    }

    return MOTEUS_OK;
}
