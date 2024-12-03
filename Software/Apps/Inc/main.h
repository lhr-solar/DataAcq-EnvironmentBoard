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
// CAN pin definitions
#define CAN1_RX GPIO_PIN_11
#define CAN1_TX GPIO_PIN_12

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
