// TODO: move both i2c and CAN to separate files

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stm32xx_hal.h"

// Initialize variables
uint8_t final_temp; // final temp from SHT45
uint8_t final_rh;   // final rh from SHT45

struct SHT45_Poll_Result // final temp/rh result of polling SHT45 sensor
{
    uint8_t final_temp;
    uint8_t final_rh;
};

StaticTask_t task_buffer;
StackType_t taskStack[configMINIMAL_STACK_SIZE];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_CAN_Init(void);

struct SHT45_Poll_Result poll_SHT45(void);

void send_payload_CAN(uint8_t final_temp, uint8_t final_rh);

static void task(void *pvParameters);
void error_handler(void);
static void success_handler(void);

int main(void)
{
    // initialize the HAL and system clock
    if (HAL_Init() != HAL_OK)
        error_handler();

    /* Initialize all configured peripherals */
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_CAN_Init();

    xTaskCreateStatic(
        task,
        "task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        taskStack,
        &task_buffer);

    vTaskStartScheduler();

    error_handler();

    return 0;
}

// for when actual (non-sensor) errors are encountered - mainly CAN transmission
void error_handler(void)
{
    __disable_irq();
    while (1)
    {
        // TODO: do smth useful here?
    }
}

// blinks LED upon successful CAN transmission
static void success_handler(void)
{
    GPIO_InitTypeDef led_init = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = GPIO_PIN_5};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_init);

    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(500);
}

static void task(void *pvParameters)
{
    while (1)
    {
        struct SHT45_Poll_Result final_SHT45_result = poll_SHT45(); // poll SHT45 sensor for temp/rh values
        final_temp = final_SHT45_result.final_temp;
        final_rh = final_SHT45_result.final_rh;

        send_payload_CAN(final_temp, final_rh); // package and send CAN payload according to environment specs
        success_handler();                      // blink LED upon successful message transmission
    }
}
