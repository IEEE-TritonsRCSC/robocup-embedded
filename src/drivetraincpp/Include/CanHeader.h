#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"
#ifdef __cplusplus
}
#endif
#include <stdint.h>

class CanHeader {
    private:

    CAN_TxHeaderTypeDef TxHeader;

    public:
    CanHeader(CAN_TxHeaderTypeDef TxHeader, uint32_t DLC, uint32_t IDE, uint32_t RTR, uint32_t StdId, FunctionalState TransmitGlobalTime);

    CAN_TxHeaderTypeDef* getTxHeaderPointer();
};
