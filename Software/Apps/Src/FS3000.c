// driver for Renesas FS3000 airflow sensor (on I2C2)
// provides functionality for airflow measurement & checksum validation

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "I2C.h"
#include "stm32xx_hal.h"
#include "SHT45.h"

// Initialize variables
I2C_HandleTypeDef hi2c2;

uint8_t airflow_high_byte; // raw airflow data from FS3000, high byte
uint8_t airflow_low_byte;  // raw airflow data from FS3000, low byte
uint16_t airflow_count;    // raw airflow data from FS3000, combined as 12-bit integer
uint8_t airflow_result;    // measured airflow value in m/s
uint8_t airflow_checksum;  // checksum for airflow received from FS3000

enum FS3000_Checksum_Result // potential values for checksum for FS3000
{
    VALID = 0,
    INVALID = 1
};

// function prototypes
static enum FS3000_Checksum_Result FS3000_Checksum(uint8_t RX_Buffer[5]);

// I2C2 initialization function
void MX_I2C2_Init(void)
{
    // init i2c parameters
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // error handling disabled since we don't know which sensors will be soldered onto each board
    // if a sensor isn't present, expected behavior is to pad with zeros (no error msg)

    HAL_I2C_Init(&hi2c2);

    /*if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        error_handler();
    }*/

    // Configure Analog filter
    HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE);

    /*if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        error_handler();
    }*/

    // Configure Digital filter
    HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0);

    /*if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
    {
        error_handler();
    }*/
}

// poll FS3000 sensor for airflow, return zeros upon error
uint8_t poll_FS3000(void)
{
    // reset airflow var
    airflow_result = 0;
    uint8_t RX_Buffer[5] = {0}; // Data received from airflow sensor

    HAL_I2C_Master_Receive(&hi2c2, (uint16_t)(0x28 << 1), RX_Buffer, sizeof RX_Buffer, 50);

    // parse received data for airflow
    // again idk how to do this useless ass datasheet
    airflow_high_byte = RX_Buffer[1];
    airflow_low_byte = RX_Buffer[2];

    airflow_count = (airflow_high_byte << 12) | airflow_low_byte;

    // some linreg to convert airflow values from datasheet to m/s
    // document this plssss
    // there is a rust driver for the FS3000 that implements an algo for this (https://github.com/JanBerktold/fs3000-rs/blob/main/src/protocol.rs)

    // handle checksum result
    enum FS3000_Checksum_Result FS3000_valid = FS3000_Checksum(RX_Buffer); // calculate and verify CRC - returns value according to enum above
    // any invalid values are sent as zeros as per DataAcq specs
    if (FS3000_valid != VALID)
    {
        airflow_result = 0;
    }

    return airflow_result;
}

// calculate and verify checksum for FS3000 Airflow sensor
static enum FS3000_Checksum_Result FS3000_Checksum(uint8_t RX_Buffer[5])
{
    enum FS3000_Checksum_Result FS3000_valid = INVALID;                      // set result to INVALID by default
    uint8_t sum = RX_Buffer[2] + RX_Buffer[3] + RX_Buffer[4] + RX_Buffer[5]; // sum of bytes 2-5 of received data (everything except checksum)

    airflow_checksum = RX_Buffer[0]; // first byte received is checksum
    airflow_checksum += sum;         // add sum of bytes 2-5 to received checksum

    // if the result of adding our sum to received checksum = 0x00, the data is valid
    if (airflow_checksum == 0x00)
    {
        FS3000_valid = VALID;
    }

    return FS3000_valid;
}
