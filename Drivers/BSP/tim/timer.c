#include "main.h"
 
extern DAC_HandleTypeDef hdac1;

uint32_t pyTick;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
uint32_t FreeRTOSRunTimeTicks;
static void MX_TIM6_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 250-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 125-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 250-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_Base_Start_IT(&htim4);
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock, uwAPB1Prescaler;

  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;

  if (TickPriority < (1UL << __NVIC_PRIO_BITS))
  {
    HAL_NVIC_SetPriority(TIM7_IRQn, TickPriority ,0U);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
    uwTickPrio = TickPriority;
  }
  else
  {
    return HAL_ERROR;
  }

  __HAL_RCC_TIM7_CLK_ENABLE();

  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  uwAPB1Prescaler = clkconfig.APB1CLKDivider;
  if (uwAPB1Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
  }

  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  htim7.Instance = TIM7;

  htim7.Init.Period = (1000000U / 1000U) - 1U;
  htim7.Init.Prescaler = uwPrescalerValue;
  htim7.Init.ClockDivision = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;

  if(HAL_TIM_Base_Init(&htim7) == HAL_OK)
  {
    return HAL_TIM_Base_Start_IT(&htim7);
  }
  return HAL_ERROR;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM7 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM7 update Interrupt */
  __HAL_TIM_DISABLE_IT(&htim7, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by Enabling TIM7 update interrupt.
  * @param  None
  * @retval None
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM7 Update interrupt */
  __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
}


void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM4)
  {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  }
  if(htim_base->Instance==TIM6)
  {
    __HAL_RCC_TIM6_CLK_DISABLE();
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM4)
  {
    __HAL_RCC_TIM4_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM4_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  }
  if(htim_base->Instance==TIM6)
	{
	   __HAL_RCC_TIM6_CLK_ENABLE();
	   HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
     HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	}
}

void bsp_InitTim(void)
{
    MX_TIM4_Init();
    MX_TIM6_Init();
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	 static uint8_t portTick;
   if(htim->Instance == TIM7)
   {
      HAL_IncTick();
   }
   if(htim->Instance == TIM4)
   {
      FreeRTOSRunTimeTicks++;			 
      pyTick++;	
		  portTick++;
		  /*检测端口拔插*/
		  if(portTick>10)
			{ 
			   ResetScanPort();	
				 portTick = 0;
			}
		   	 
   }
   if(htim->Instance == TIM6)
   {
      ;
   }
}

TIM_HandleTypeDef *get_dac_time6_handle(void)
{
   return &htim6;
}

void TIM4_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim4);
}
void TIM7_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim7);
}
void TIM6_DAC_IRQHandler(void)
{
  HAL_DAC_IRQHandler(&hdac1);
  HAL_TIM_IRQHandler(&htim6);
}
void ConfigureTimeForRunTimeStats(void)
{
	#if 1
TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* 开启 TIM5 时钟（通常由 CubeMX 自动生成） */
    __HAL_RCC_TIM5_CLK_ENABLE();

    htim5.Instance = TIM5;
    /* 计算预分频器：假设 APB1 时钟 = 84MHz，预分频 83 得 1MHz 计数频率 */
    htim5.Init.Prescaler = 250- 1;          // 84MHz / 84 = 1MHz
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 0xFFFFFFFF;          // 最大周期，自由运行
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* 启动定时器，但不启用中断 */
    HAL_TIM_Base_Start(&htim5);
		#endif
}
unsigned long GetRunTimeCounterValue(void)
{
	  #if 1
    return __HAL_TIM_GET_COUNTER(&htim5);
	  #else
	  return FreeRTOSRunTimeTicks;
	  #endif
}
