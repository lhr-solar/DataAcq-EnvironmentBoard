// driver for Sensirion SHT45 temperature & relative humidity sensor (on I2C1)
// provides functionality for temp/rh measurement & checksum validation

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "I2C.h"
#include "stm32xx_hal.h"
#include "SHT45.h"

// Initialize variables
I2C_HandleTypeDef hi2c1;

uint8_t TX_Buffer[1] = {0xFD}; // Data to send to request reading from humidity sensor

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

struct SHT45_Poll_Result // final temp/rh result of polling SHT45 sensor to return
{
    uint8_t final_temp;
    uint8_t final_rh;
};

// function prototypes
static enum SHT45_CRC_Result SHT45_CRC(uint8_t RX_Buffer[6]);

// I2C1 initialization function
void MX_I2C1_Init(void)
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

// poll SHT45 sensor for temperature and relative humidity, return zeros upon error
struct SHT45_Poll_Result poll_SHT45(void)
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

    struct SHT45_Poll_Result r = {temp_degC, rh_percentRH};
    return r;
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
