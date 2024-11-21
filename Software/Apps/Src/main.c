/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "string.h"
#include "CAN.h"
#include "stm32xx_hal.h"

I2C_HandleTypeDef hi2c1;

// Initialize variables
uint8_t TX_Buffer[1] = {0xFD}; // Data to send to request reading from humidity sensor
uint8_t RX_Buffer[6];          // Data received from humidity sensor
uint8_t tx_data[8];

uint16_t temp_ticks;
uint16_t temp_degC;
uint16_t rh_ticks;
uint16_t rh_percentRH;

StaticTask_t task_buffer;
StackType_t taskStack[configMINIMAL_STACK_SIZE];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_CAN_Init(void);
static void task(void *pvParameters);
static void error_handler(void);
static void success_handler(void);

int main(void)
{
    // initialize the HAL and system clock
    if (HAL_Init() != HAL_OK)
        error_handler();

    /* Initialize all configured peripherals */
    MX_I2C1_Init();
    MX_CAN_Init();

    xTaskCreateStatic(
        task,
        "task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        taskStack,
        &task_buffer);

    vTaskStartScheduler();

    error_handler();

    /*  while (1)
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
      */

    return 0;
}

// CAN initialization function
static void MX_CAN_Init(void)
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

// I2C initialization function
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
            error_handler();
        }*/

    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);

    /** Configure Analogue filter
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        error_handler();
    }*/

    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);

    /** Configure Digital filter
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        error_handler();
    }*/
}

static void error_handler(void)
{
    while (1)
    {
    }
}

static void success_handler(void)
{
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_init);

    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
}

static void task(void *pvParameters)
{
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
        temp_degC = (-45 + (175 * temp_ticks / 65535));

        // parse received data for relative humidity
        rh_ticks = (RX_Buffer[3] * 256) + RX_Buffer[4];
        rh_percentRH = (-6 + (125 * rh_ticks / 65535));

        // create payload to send
        CAN_TxHeaderTypeDef tx_header = {0};
        tx_header.StdId = 0x200;
        tx_header.RTR = CAN_RTR_DATA;
        tx_header.IDE = CAN_ID_STD;
        tx_header.DLC = 2;
        tx_header.TransmitGlobalTime = DISABLE;

        // send temperature/humidity payloads to 0x1
        tx_data[0] = temp_degC;
        tx_data[1] = rh_percentRH;

        if (can_send(hcan1, &tx_header, tx_data, true) != CAN_SENT)
            error_handler();

        success_handler();
    }
}
