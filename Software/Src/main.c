#include "stm32xx_hal.h"

int main(){
    HAL_Init();

    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_0
    };
    
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable clock for GPIOA
    HAL_GPIO_Init(GPIOB, &led_config); // initialize GPIOA with led_config

    while(1){
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
        HAL_Delay(200);
    }

    return 0;
}