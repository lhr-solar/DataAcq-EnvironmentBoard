// test sending a CAN message with the DataAcq Environment format

/* Includes ------------------------------------------------------------------*/
#include "CAN_test.h"

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;
CAN_FilterTypeDef canfilterconfig;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxHeader; // CAN transmit header info structure
uint8_t TxData[8];            // data to transmit
uint32_t TxMailbox;           // mailbox (storage space for messages)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_CAN1_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Initialize all configured peripherals */
    MX_CAN1_Init();

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {

        // no idea what this does but for testing....
        TxHeader.IDE = CAN_ID_STD;
        TxHeader.StdId = 0x200;
        TxHeader.RTR = CAN_RTR_DATA;
        TxHeader.DLC = 2;

        TxData[0] = 50;
        TxData[1] = 0xAA;

        // this has the dumbass error handling, may have to fix
        if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

/**
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void)
{
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 16;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_14TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_6TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    HAL_CAN_Init(&hcan1);

    /* need to impl proper error handling...
        if (HAL_CAN_Init(&hcan1) != HAL_OK)
        {
            Error_Handler();
        }*/

    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 18; // which filter bank to use from the assigned ones
    canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilterconfig.FilterIdHigh = 0x200 << 5;
    canfilterconfig.FilterIdLow = 0;
    canfilterconfig.FilterMaskIdHigh = 0x200 << 5;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 20; // how many filters to assign to the CAN1 (master can)

    HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
