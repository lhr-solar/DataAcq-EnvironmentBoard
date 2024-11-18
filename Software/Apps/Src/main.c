/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"

I2C_HandleTypeDef hi2c1;
CAN_HandleTypeDef hcan1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

uint8_t TX_Buffer[1] = {0xFD}; // DATA to send
uint8_t RX_Buffer[6];          // DATA to receive
uint16_t temp_ticks;
uint16_t temp_degC;
uint16_t rh_ticks;
uint16_t rh_percentRH;

CAN_HandleTypeDef hcan1;
CAN_TxHeaderTypeDef pHeader;   // declare a specific header for message transmittions
CAN_RxHeaderTypeDef pRxHeader; // declare header for message reception
uint32_t TxMailbox;
uint8_t a, r;                    // declare byte to be transmitted //declare a receive byte
CAN_FilterTypeDef sFilterConfig; // declare CAN filter structure

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_CAN1_Init(void);
static void MX_GPIO_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable clock for GPIOA
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable clock for GPIOB

    /* Initialize all configured peripherals */
    MX_I2C1_Init();
    MX_CAN1_Init();
    MX_GPIO_Init();

    // TESTING CAN HERE......
    pHeader.DLC = 1;            // give message size of 1 byte
    pHeader.IDE = CAN_ID_STD;   // set identifier to standard
    pHeader.RTR = CAN_RTR_DATA; // set data type to remote transmission request?
    pHeader.StdId = 0x244;      // define a standard identifier, used for message identification by filters (switch this for the other microcontroller)

    // filter one (stack light blink)
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0; // set fifo assignment
    sFilterConfig.FilterIdHigh = 0x245 << 5;               // the ID that the filter looks for (switch this for the other microcontroller)
    sFilterConfig.FilterIdLow = 0;
    sFilterConfig.FilterMaskIdHigh = 0;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // set filter scale
    sFilterConfig.FilterActivation = ENABLE;

    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig); // configure CAN filter

 //   HAL_CAN_Start(&hcan1);                                             // start CAN
 //   HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING); // enable interrupts

    while (1)
    {
        // reset temp/rh vars
        temp_ticks = 0;
        rh_ticks = 0;

        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(0x44 << 1), TX_Buffer, sizeof TX_Buffer, 1000); // Sending in Blocking mode
        HAL_Delay(10);                                                                             // wait for 0.01 seconds (datasheet value) for response from sensor
        HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(0x44 << 1), RX_Buffer, sizeof RX_Buffer, 50);

        // TODO: Implement checksum to verify data is read correctly - this will be important when we use the breakout boards as that may introduce noise on i2c bus

        // parse received data for temperature
        temp_ticks = (RX_Buffer[0] * 256) + RX_Buffer[1];
        temp_degC = -45 + (175 * temp_ticks / 65535);

        // parse received data for relative humidity
        rh_ticks = (RX_Buffer[3] * 256) + RX_Buffer[4];
        rh_percentRH = -6 + (125 * rh_ticks / 65535);
    }

    return 0;
}

/* CAN1 init function */
static void MX_CAN1_Init(void)
{

    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 21;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    // may need to disable error handling here if same bullshit happening
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{
    // init i2c parameters
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // TODO: all errors are currently being ignored bc it always throws? for some reason
    // need to fix with proper error handling

    HAL_I2C_Init(&hi2c1);

    /*    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        {
            Error_Handler();
        }*/

    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);

    /** Configure Analogue filter
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }*/

    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);

    /** Configure Digital filter
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }*/
}

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

    /*Configure GPIO pin : PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        /* TODO: add some useful error message for when HAL error occurs */
    }
}
