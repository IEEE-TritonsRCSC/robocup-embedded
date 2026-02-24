#include "CanHeader.h"
#include "helpers.h"

CanHeader::CanHeader(CAN_TxHeaderTypeDef TxHeader, uint32_t DLC, uint32_t IDE, uint32_t RTR, uint32_t StdId, FunctionalState TransmitGlobalTime)
{
    this->TxHeader = TxHeader;
    this->TxHeader.DLC = DLC;
    this->TxHeader.IDE = IDE;
    this->TxHeader.RTR = RTR;
    this->TxHeader.StdId = StdId;
    this->TxHeader.TransmitGlobalTime = TransmitGlobalTime;
}

CAN_TxHeaderTypeDef* CanHeader::getTxHeaderPointer() {
    return &TxHeader;
}