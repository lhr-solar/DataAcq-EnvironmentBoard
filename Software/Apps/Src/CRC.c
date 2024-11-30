// Init/Deinit functions for CRC

/* Includes ------------------------------------------------------------------*/
/*
#include "main.h"
#include "stm32xx_hal.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
}

void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    if (hcrc->Instance == CRC)
    {
        __HAL_RCC_CRC_CLK_ENABLE();
    }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
    if (hcrc->Instance == CRC)
    {
        __HAL_RCC_CRC_CLK_DISABLE();
    }
}
*/