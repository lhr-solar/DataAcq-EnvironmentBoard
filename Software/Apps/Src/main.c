// TODO: move both i2c and CAN to separate files

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include "string.h"
#include "CAN.h"
#include "stm32xx_hal.h"

// Initialize variables
uint8_t tx_data[8]; // CAN transmission payload
uint8_t final_temp; // final temp from SHT45
uint8_t final_rh;   // final rh from SHT45

struct SHT45_Poll_Result // final temp/rh result of polling SHT45 sensor
{
    uint8_t final_temp;
    uint8_t final_rh;
};

StaticTask_t task_buffer;
StackType_t taskStack[configMINIMAL_STACK_SIZE];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
static void MX_CAN_Init(void);

struct SHT45_Poll_Result poll_SHT45(void);

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
    MX_I2C2_Init();
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

// for when actual (non-sensor) errors are encountered - mainly CAN transmission
static void error_handler(void)
{
    __disable_irq();
    while (1)
    {
        // TODO: do smth useful here?
    }
}

// package and send CAN payload according to environment specs
static void send_payload_CAN()
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

// blinks LED upon successful CAN transmission
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
    // TODO: organize into functions for humidity, airflow, checksum, pack/send msg, etc

    while (1)
    {
        struct SHT45_Poll_Result final_SHT45_result = poll_SHT45(); // poll SHT45 sensor for temp/rh values
        final_temp = final_SHT45_result.final_temp;
        final_rh = final_SHT45_result.final_rh;

        send_payload_CAN(); // package and send CAN payload according to environment specs
        success_handler();  // blink LED upon successful message transmission
    }
}
