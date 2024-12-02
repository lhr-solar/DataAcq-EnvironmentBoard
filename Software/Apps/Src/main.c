/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include "string.h"
#include "CAN.h"
#include "stm32xx_hal.h"

I2C_HandleTypeDef hi2c1;
// CRC_HandleTypeDef hcrc;

// Initialize variables
uint8_t TX_Buffer[1] = {0xFD}; // Data to send to request reading from humidity sensor
uint8_t tx_data[8];            // CAN transmission payload

uint16_t temp_ticks;  // raw temp data received from SHT45
uint8_t temp_degC;    // temp value in celsius
uint16_t rh_ticks;    // raw rh data received from SHT45
uint8_t rh_percentRH; // rh value as percentage

uint16_t checksum_temp;            // checksum for temp received from SHT45
uint16_t checksum_rh;              // checksum for rh receievd from SHT45
const uint16_t crc_poly = 0x31;    // polynomial used to calculate checksum for SHT45 (from datasheet)
const uint16_t crc_initial = 0xFF; // initial CRC value for SHT45 (from datasheet)

enum SHT45_CRC_Result // potential values for CRC result for SHT45
{
    VALID = 0,
    TEMP_VALID = 1,
    RH_VALID = 2,
    INVALID = 3
};

StaticTask_t task_buffer;
StackType_t taskStack[configMINIMAL_STACK_SIZE];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_CAN_Init(void);

static void task(void *pvParameters);
static void error_handler(void);
static void success_handler(void);
static enum SHT45_CRC_Result SHT45_CRC(uint8_t RX_Buffer[6]);

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

    // error handling disabled since we don't know which sensors will be soldered onto each board
    // if a sensor isn't present, expected behavior is to pad with zeros (no error msg)

    HAL_I2C_Init(&hi2c1);

    /*if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        error_handler();
    }*/

    // Configure Analog filter
    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);

    /*if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        error_handler();
    }*/

    // Configure Digital filter
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);

    /*if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        error_handler();
    }*/
}

// for when actual errors are encountered - mainly CAN transmission
static void error_handler(void)
{
    __disable_irq();
    while (1)
    {
        // TODO: do smth useful here?
    }
}

// poll SHT45 sensor for temperature and relative humidity, return zeros upon error
static void poll_SHT45()
{
    // reset temp/rh vars
    temp_ticks = 0;
    rh_ticks = 0;
    uint8_t RX_Buffer[6] = {0}; // Data received from humidity sensor

    HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(0x44 << 1), TX_Buffer, sizeof TX_Buffer, 1000); // Sending in Blocking mode
    HAL_Delay(10);                                                                             // wait for 0.01 seconds (datasheet value) for response from sensor
    HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(0x44 << 1), RX_Buffer, sizeof RX_Buffer, 50);

    // parse received data for temperature
    temp_ticks = (RX_Buffer[0] * 256) + RX_Buffer[1];
    temp_degC = (-45 + (175 * temp_ticks / 65535));

    // parse received data for relative humidity
    rh_ticks = (RX_Buffer[3] * 256) + RX_Buffer[4];
    rh_percentRH = (-6 + (125 * rh_ticks / 65535));

    // if percent rh is < 0, set to 0. if percent rh is > 100, set to 100 (expected behavior from SHT45 datasheet)
    if (rh_percentRH < 0)
    {
        rh_percentRH = 0;
    }
    else if (rh_percentRH > 100)
    {
        rh_percentRH = 100;
    }

    // handle CRC result
    enum SHT45_CRC_Result SHT45_valid = SHT45_CRC(RX_Buffer); // calculate and verify CRC - returns value according to enum above
    // any invalid values are sent as zeros as per DataAcq specs
    if (SHT45_valid == TEMP_VALID)
    {
        rh_percentRH = 0;
    }
    else if (SHT45_valid == RH_VALID)
    {
        temp_degC = 0;
    }
    else if (SHT45_valid == INVALID)
    {
        temp_degC = 0;
        rh_percentRH = 0;
    }
}

// calculate and verify CRC-8 for the SHT45 humidity sensor
static enum SHT45_CRC_Result SHT45_CRC(uint8_t RX_Buffer[6])
{
    checksum_temp = 0;
    checksum_rh = 0;
    uint8_t computed_temp_crc;
    uint8_t computed_rh_crc;
    enum SHT45_CRC_Result SHT45_valid = INVALID; // set result to INVALID by default

    // use checksum to verify data is received correctly - important when we use the breakout boards as that may introduce noise on i2c bus
    checksum_temp = RX_Buffer[2];

    // TEST: manually set known values... checksum of 0xBEEF = 0x92 (according to SHT45 datasheet)
    // RX_Buffer[0] = 0xBE;
    // RX_Buffer[1] = 0xEF;
    // checksum_temp = 0x92;

    // manually implemented using algo at https://www.st.com/resource/en/application_note/an4187-using-the-crc-peripheral-on-stm32-microcontrollers-stmicroelectronics.pdf
    // sequentially (thanks chatgpt :D) - do the crc for first byte first and then use that val as input data for the crc for second byte and should give you proper value, verify this with what we actually recieve from the sensor

    computed_temp_crc = crc_initial ^ RX_Buffer[0]; // 1st step of CRC for 1st temp byte

    // calc crc for first byte
    for (uint8_t bindex = 0; bindex < sizeof(RX_Buffer[0]) * 8; bindex++)
    {
        uint8_t MSB = (int8_t)computed_temp_crc < 0; // get MSB of CRC

        if (MSB == 1)
        {
            computed_temp_crc = (computed_temp_crc << 1) ^ crc_poly; // shift one bit left, XOR with poly
        }
        else
        {
            computed_temp_crc = computed_temp_crc << 1; // shift one bit left
        }
    }

    // sequential calc for second byte (using first byte crc as initial crc)
    computed_temp_crc = computed_temp_crc ^ RX_Buffer[1]; // 1st step of CRC for 2nd temp byte

    for (uint8_t bindex = 0; bindex < sizeof(RX_Buffer[1]) * 8; bindex++)
    {
        uint8_t MSB = (int8_t)computed_temp_crc < 0; // get MSB of CRC

        if (MSB == 1)
        {
            computed_temp_crc = (computed_temp_crc << 1) ^ crc_poly; // shift one bit left, XOR with poly
        }
        else
        {
            computed_temp_crc = computed_temp_crc << 1; // shift one bit left
        }
    }

    // same calulcation for rh bytes
    checksum_rh = RX_Buffer[5];

    computed_rh_crc = crc_initial ^ RX_Buffer[3]; // 1st step of CRC for 1st rh byte

    // calc crc for first byte
    for (uint8_t bindex = 0; bindex < sizeof(RX_Buffer[3]) * 8; bindex++)
    {
        uint8_t MSB = (int8_t)computed_rh_crc < 0; // get MSB of CRC

        if (MSB == 1)
        {
            computed_rh_crc = (computed_rh_crc << 1) ^ crc_poly; // shift one bit left, XOR with poly
        }
        else
        {
            computed_rh_crc = computed_rh_crc << 1; // shift one bit left
        }
    }

    // sequential calc for second byte (using first byte crc as initial crc)
    computed_rh_crc = computed_rh_crc ^ RX_Buffer[4]; // 1st step of CRC for 2nd rh byte

    for (uint8_t bindex = 0; bindex < sizeof(RX_Buffer[4]) * 8; bindex++)
    {
        uint8_t MSB = (int8_t)computed_rh_crc < 0; // get MSB of CRC

        if (MSB == 1)
        {
            computed_rh_crc = (computed_rh_crc << 1) ^ crc_poly; // shift one bit left, XOR with poly
        }
        else
        {
            computed_rh_crc = computed_rh_crc << 1; // shift one bit left
        }
    }

    // check if computed crc bytes match received bytes
    if (computed_temp_crc == checksum_temp)
    {
        if (computed_rh_crc == checksum_rh)
        {
            SHT45_valid = VALID;
        }
        else
        {
            SHT45_valid = TEMP_VALID;
        }
    }
    else if (computed_rh_crc == checksum_rh)
    {
        SHT45_valid = RH_VALID;
    }
    else
    {
        SHT45_valid = INVALID;
    }

    return SHT45_valid;
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

    // payload format (bytes) - TODO: make sure this goes in docs somewhere:
    // 0 - Temperature reading from SHT45 (Degrees C)
    // 1 - Relative Humidity reading from SHT45 (%RH)
    // 2 - Padding zeros
    // 3 - Airflow reading from FS-3000 (m/s)
    // 4 - Padding zeros
    // 5 - Temperature reading from LMT87 (Degrees C)
    // 6 - Padding zeros
    // 7 - Padding zeros

    // set payloads
    tx_data[0] = temp_degC;
    tx_data[1] = rh_percentRH;

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
        poll_SHT45();       // poll SHT45 sensor for temp/rh values
        send_payload_CAN(); // package and send CAN payload according to environment specs
        success_handler();  // blink LED upon successful message transmission
    }
}
