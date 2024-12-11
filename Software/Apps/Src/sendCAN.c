// driver for STM32 CAN peripheral
// provides functions to initialize CAN bus and package/send sensor data

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "CAN.h"
#include "stm32xx_hal.h"

// Initialize variables
uint8_t tx_data[8]; // CAN transmission payload

// function prototypes
void error_handler(void);
void MX_CAN_Init(void);

// CAN initialization function
void MX_CAN_Init(void)
{
    // create filter
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    // setup CAN init
    hcan1->Init.Prescaler = 16;
    hcan1->Init.Mode = CAN_MODE_LOOPBACK; // change this to CAN_MODE_NORMAL for production (need terminating resistors at ends)
    hcan1->Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1->Init.TimeSeg1 = CAN_BS1_14TQ;
    hcan1->Init.TimeSeg2 = CAN_BS2_6TQ;
    hcan1->Init.TimeTriggeredMode = DISABLE;
    hcan1->Init.AutoBusOff = DISABLE;
    hcan1->Init.AutoWakeUp = DISABLE;
    hcan1->Init.AutoRetransmission = ENABLE;
    hcan1->Init.ReceiveFifoLocked = DISABLE;
    hcan1->Init.TransmitFifoPriority = DISABLE;

    // initialize CAN
    if (can_init(hcan1, &sFilterConfig) != CAN_OK)
        error_handler();
}

// package and send CAN payload according to environment specs
void send_payload_CAN(uint8_t final_temp, uint8_t final_rh)
{
    // create CAN payload to send data on DataAcq CAN bus
    CAN_TxHeaderTypeDef tx_header = {0};
    tx_header.StdId = 0x200; // base address of environment board (starts at x200) - TODO: put in header file for portability?
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 2;
    tx_header.TransmitGlobalTime = DISABLE;

    // payload format (bytes) - TODO: make sure this goes in docs somewhere
    // 0 - Temperature reading from SHT45 (Degrees C)
    // 1 - Relative Humidity reading from SHT45 (%RH)
    // 2 - Padding zeros
    // 3 - Airflow reading from FS-3000 (m/s)
    // 4 - Padding zeros
    // 5 - Temperature reading from LMT87 (Degrees C)
    // 6 - Padding zeros
    // 7 - Padding zeros

    // set payloads
    tx_data[0] = final_temp;
    tx_data[1] = final_rh;

    if (can_send(hcan1, &tx_header, tx_data, true) != CAN_SENT)
        error_handler();
}
