/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private defines -----------------------------------------------------------*/
// I2C pin definitions (for nucleo-f446re - may need to change/make adaptable?)
#define I2C1_SCL GPIO_PIN_8
#define I2C1_SDA GPIO_PIN_9
// CAN pin definitions
#define CAN1_RX GPIO_PIN_11
#define CAN1_TX GPIO_PIN_12

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
