/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include <nucleo_hal_bsp.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim3;
/* Captured Values */
uint32_t               uwIC2Value1 = 0;
uint32_t               uwIC2Value2 = 0;
uint32_t               uwDiffCapture = 0;

/* Capture index */
uint16_t               uhCaptureIndex = 0;

/* Frequency Value */
uint32_t               uwFrequency = 0;
void MX_TIM3_Init(void);

int main(void) {
  HAL_Init();

  Nucleo_BSP_Init();
  MX_TIM3_Init();

  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 47999; //48MHz/48000 = 1000Hz
  htim6.Init.Period = 499; //1000HZ / 500 = 2Hz = 0.5s

  __TIM6_CLK_ENABLE();

  HAL_NVIC_SetPriority(TIM6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM6_IRQn);

  HAL_TIM_Base_Init(&htim6);
  HAL_TIM_Base_Start_IT(&htim6);

  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);

  while (1) {
    if(uwFrequency != 0) {

      while(1);
    }
  }
}



void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
{
    if(uhCaptureIndex == 0)
    {
      /* Get the 1st Input Capture value */
      uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      uhCaptureIndex = 1;
}
    else if(uhCaptureIndex == 1)
    {
      /* Get the 2nd Input Capture value */
      uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

      /* Capture computation */
      if (uwIC2Value2 > uwIC2Value1)
      {
        uwDiffCapture = (uwIC2Value2 - uwIC2Value1);
      }
      else if (uwIC2Value2 < uwIC2Value1)
      {
        uwDiffCapture = ((0xFFFF - uwIC2Value1) + uwIC2Value2);
      }
      else
      {
        /* If capture values are equal, we have reached the limit of frequency
           measures */
        asm("BKPT #0");
      }
      /* Frequency computation: for this example TIMx (TIM1) is clocked by
         APB1Clk */
      uwFrequency = HAL_RCC_GetPCLK1Freq() / uwDiffCapture;
      uhCaptureIndex = 0;
      char msg[20];
      sprintf(msg, "T1: %u\r\n", uwIC2Value2);
      HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

      sprintf(msg, "T2: %u\r\n", uwIC2Value2);
      HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);

      sprintf(msg, "Speed: %u\r\n", uwIC2Value2-uwIC2Value1);
      HAL_UART_Transmit(&huart2, msg, strlen(msg), HAL_MAX_DELAY);
    }
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim_base->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }

}

/* TIM3 init function */
void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim3);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

  HAL_TIM_IC_Init(&htim3);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

}

void TIM3_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim3);
}

void TIM6_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim6);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(htim->Instance == TIM6)
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/