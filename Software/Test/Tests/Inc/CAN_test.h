/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_TEST_H
#define __CAN_TEST_H

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
#define CAN1_RX GPIO_PIN_11
#define CAN1_TX GPIO_PIN_12

#ifdef __cplusplus
}
#endif

#endif