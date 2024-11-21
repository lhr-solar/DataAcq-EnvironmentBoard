/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_TEST_H
#define __I2C_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
// I2C pin definitions (for nucleo - may need to change/make adaptable?)
#define I2C1_SCL GPIO_PIN_8
#define I2C1_SDA GPIO_PIN_9

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

#ifdef __cplusplus
}
#endif

#endif