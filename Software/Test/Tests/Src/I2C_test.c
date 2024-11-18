// send test data to i2c sensor and check response

/* Includes ------------------------------------------------------------------*/
#include "I2C_test.h"
#include "string.h"

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
uint8_t TX_Buffer[1] = {0xFD}; // DATA to send
uint8_t RX_Buffer[6];          // DATA to receive
uint16_t temp_ticks;
uint16_t temp_degC;
uint16_t rh_ticks;
uint16_t rh_percentRH;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Initialize all configured peripherals */
    MX_I2C1_Init();

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

        // TODO: do the <0 and >100 stuff from datasheet
    }

    return 0;
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

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C1 GPIO Configuration
        PB8     ------> I2C1_SCL
        PB9     ------> I2C1_SDA
        */
        GPIO_InitStruct.Pin = I2C1_SCL | I2C1_SDA;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
    }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PB8     ------> I2C1_SCL
        PB9     ------> I2C1_SDA
        */
        HAL_GPIO_DeInit(GPIOB, I2C1_SCL);

        HAL_GPIO_DeInit(GPIOB, I2C1_SDA);
    }
}
