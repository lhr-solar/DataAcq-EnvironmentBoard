// Init/Deinit functions for I2C1 and I2C2

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "I2C.h"
#include "stm32xx_hal.h"

I2C_HandleTypeDef hi2c2;

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

  if (hi2c->Instance == I2C2)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB3      ------> I2C2_SDA
    */
    GPIO_InitStruct.Pin = I2C2_SCL | I2C2_SDA;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  }
}

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

  if (hi2c->Instance == I2C2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB3      ------> I2C2_SDA
    */
    HAL_GPIO_DeInit(GPIOB, I2C2_SCL);

    HAL_GPIO_DeInit(GPIOB, I2C2_SDA);
  }
}

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