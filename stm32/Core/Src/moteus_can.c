/**
 * @file moteus_can.c
 * @brief HAL FDCAN wrapper for Moteus communication
 *
 * Provides initialization and TX/RX functions for STM32 FDCAN peripheral.
 * Configured for use with Moteus motor controllers.
 *
 * NOTE: Bit timing is configured in CubeMX, not here. This file only handles
 * filter setup, starting the peripheral, and enabling interrupts.
 */

#include "moteus.h"
#include <string.h>

/* ============================================================================
 * FDCAN Initialization
 *
 * Bit timing should be configured in CubeMX. For 1 Mbps with 170 MHz clock:
 *   Prescaler=1, TimeSeg1=135, TimeSeg2=34, SyncJumpWidth=34
 *
 * This function only:
 *   - Configures RX filter for Moteus responses
 *   - Starts the FDCAN peripheral
 *   - Enables RX interrupt
 * ============================================================================ */

HAL_StatusTypeDef moteus_can_init(FDCAN_HandleTypeDef* hfdcan)
{
    if (hfdcan == NULL) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status;

    /* Configure filter to accept Moteus responses (extended IDs) */
    /* We accept all extended IDs and filter in software */
    FDCAN_FilterTypeDef filter;
    filter.IdType = FDCAN_EXTENDED_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = 0x00000000;  /* Match pattern */
    filter.FilterID2 = 0x00000000;  /* Mask: accept all extended IDs */

    status = HAL_FDCAN_ConfigFilter(hfdcan, &filter);
    if (status != HAL_OK) {
        return status;
    }

    /* Configure global filter to accept all non-matching frames.
     * Moteus responses may come as standard IDs even though Moteus uses
     * extended IDs (the hardware reports small IDs like 0x100 as standard).
     * Accept both standard and extended into FIFO0 for software filtering. */
    status = HAL_FDCAN_ConfigGlobalFilter(hfdcan,
                                          FDCAN_ACCEPT_IN_RX_FIFO0,  /* NonMatchingStd */
                                          FDCAN_ACCEPT_IN_RX_FIFO0,  /* NonMatchingExt */
                                          FDCAN_REJECT_REMOTE,       /* RejectRemoteStd */
                                          FDCAN_REJECT_REMOTE);      /* RejectRemoteExt */
    if (status != HAL_OK) {
        return status;
    }

    /* Start FDCAN */
    status = HAL_FDCAN_Start(hfdcan);
    if (status != HAL_OK) {
        return status;
    }

    /* Enable RX interrupt notification */
    status = HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    if (status != HAL_OK) {
        return status;
    }

    return status;
}

/* ============================================================================
 * Transmit Functions
 * ============================================================================ */

/**
 * @brief Convert data length to FDCAN DLC code
 */
static uint32_t get_fdcan_dlc(uint8_t len)
{
    if (len <= 8) return len;
    if (len <= 12) return FDCAN_DLC_BYTES_12;
    if (len <= 16) return FDCAN_DLC_BYTES_16;
    if (len <= 20) return FDCAN_DLC_BYTES_20;
    if (len <= 24) return FDCAN_DLC_BYTES_24;
    if (len <= 32) return FDCAN_DLC_BYTES_32;
    if (len <= 48) return FDCAN_DLC_BYTES_48;
    return FDCAN_DLC_BYTES_64;
}

/**
 * @brief Get actual data length from FDCAN DLC code
 */
static uint8_t get_data_length(uint32_t dlc)
{
    if (dlc <= 8) return dlc;

    switch (dlc) {
        case FDCAN_DLC_BYTES_12: return 12;
        case FDCAN_DLC_BYTES_16: return 16;
        case FDCAN_DLC_BYTES_20: return 20;
        case FDCAN_DLC_BYTES_24: return 24;
        case FDCAN_DLC_BYTES_32: return 32;
        case FDCAN_DLC_BYTES_48: return 48;
        case FDCAN_DLC_BYTES_64: return 64;
        default: return 8;
    }
}

HAL_StatusTypeDef moteus_can_transmit(FDCAN_HandleTypeDef* hfdcan,
                                       const moteus_can_frame_t* frame)
{
    if (hfdcan == NULL || frame == NULL) {
        return HAL_ERROR;
    }

    FDCAN_TxHeaderTypeDef tx_header;
    tx_header.Identifier = frame->id;
    tx_header.IdType = FDCAN_EXTENDED_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = get_fdcan_dlc(frame->len);
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = frame->brs ? FDCAN_BRS_ON : FDCAN_BRS_OFF;
    tx_header.FDFormat = frame->is_fd ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    /* Pad data to DLC length */
    uint8_t tx_data[64];
    memset(tx_data, 0, sizeof(tx_data));
    memcpy(tx_data, frame->data, frame->len);

    return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, tx_data);
}

/* ============================================================================
 * Receive Functions
 * ============================================================================ */

HAL_StatusTypeDef moteus_can_receive(FDCAN_HandleTypeDef* hfdcan,
                                      moteus_can_frame_t* frame,
                                      uint32_t timeout_ms)
{
    if (hfdcan == NULL || frame == NULL) {
        return HAL_ERROR;
    }

    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[64];

    /* Check if message available */
    uint32_t fill_level = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0);
    if (fill_level == 0) {
        if (timeout_ms == 0) {
            return HAL_TIMEOUT;
        }

        /* Poll for message */
        uint32_t start = HAL_GetTick();
        while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) == 0) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                return HAL_TIMEOUT;
            }
        }
    }

    /* Get message */
    HAL_StatusTypeDef status = HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data);
    if (status != HAL_OK) {
        return status;
    }

    /* Convert to moteus frame */
    frame->id = rx_header.Identifier;
    frame->len = get_data_length(rx_header.DataLength);
    frame->is_fd = (rx_header.FDFormat == FDCAN_FD_CAN);
    frame->brs = (rx_header.BitRateSwitch == FDCAN_BRS_ON);
    memcpy(frame->data, rx_data, frame->len);

    return HAL_OK;
}

/* ============================================================================
 * Helper function to process RX in interrupt context
 * ============================================================================ */

void moteus_fdcan_rx_callback(FDCAN_HandleTypeDef* hfdcan)
{
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[64];

    /* Get all available messages */
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0) {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            /* Process the frame */
            uint8_t len = get_data_length(rx_header.DataLength);
            moteus_process_rx(&rx_header, rx_data, len);
        }
    }
}
