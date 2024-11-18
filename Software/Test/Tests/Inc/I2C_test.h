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

#ifdef __cplusplus
}
#endif

#endif