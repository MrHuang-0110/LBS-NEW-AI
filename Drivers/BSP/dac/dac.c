#include "main.h"

extern TIM_HandleTypeDef htim6;
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;
static SemaphoreHandle_t dacCompleteSemaphore;

static void MX_DMA_Init(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 8, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}
static void MX_DAC1_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdac->Instance==DAC1)
  {

    __HAL_RCC_DAC12_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC1 GPIO Configuration
    PA4     ------> DAC1_OUT1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* DAC1 DMA Init */
    /* DAC1_CH1 Init */
    hdma_dac1_ch1.Instance = DMA1_Stream2;
    hdma_dac1_ch1.Init.Request = DMA_REQUEST_DAC1;
    hdma_dac1_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac1_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac1_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac1_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_dac1_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_dac1_ch1.Init.Mode = DMA_NORMAL;
    hdma_dac1_ch1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dac1_ch1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dac1_ch1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdac,DMA_Handle1,hdma_dac1_ch1);

    /* DAC1 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  }

}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC1)
  {
    __HAL_RCC_DAC12_CLK_DISABLE();
    /**DAC1 GPIO Configuration
    PA4     ------> DAC1_OUT1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
    /* DAC1 DMA DeInit */
    HAL_DMA_DeInit(hdac->DMA_Handle1);
  }
}

void bsp_InitDac(void)
{
    MX_DMA_Init();
    MX_DAC1_Init();
	
	  dacCompleteSemaphore = xSemaphoreCreateBinary();
}

void keyPlayerVIOC(void)
{
	 BSP_KEY *key = getKeyHandle();
	 HAL_TIM_Base_Start_IT(&htim6); 
	 HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 	 
   dac_out_put(key->keyMusic,key->keyMusicSize - 6144);	  
   HAL_TIM_Base_Stop_IT(&htim6);
   HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);		
}

void LinkePlayerVIOC(void)
{ 
   BSP_KEY *key = getKeyHandle();
   HAL_TIM_Base_Start_IT(&htim6); 
   HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 	 
   dac_out_put(key->LinkeMusic,key->LinkeMusicSize - 6144);	  
   HAL_TIM_Base_Stop_IT(&htim6);
   HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);	    
}
void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_dac1_ch1);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dacCompleteSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
 
void dac_out_put(unsigned char *DATA,int Length)
{
    #if 0
    HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_1, (uint32_t *)DATA, Length, DAC_ALIGN_8B_R);
	   
    while(HAL_DAC_GetState(&hdac1) != HAL_DAC_STATE_READY)
    {
      if(HAL_DAC_GetState(&hdac1) == HAL_DAC_STATE_ERROR)
           break;
    }
    HAL_DAC_Stop_DMA(&hdac1, DAC1_CHANNEL_1);
	#else
		HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_1, (uint32_t *)DATA, Length, DAC_ALIGN_8B_R);
    
    // 等待信号量，但允许任务切换
    if(xSemaphoreTake(dacCompleteSemaphore, portMAX_DELAY) == pdTRUE) {
        HAL_DAC_Stop_DMA(&hdac1, DAC1_CHANNEL_1);
    }
	#endif
}
