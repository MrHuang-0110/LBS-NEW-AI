#include "uart.h"
#include "string.h"
//#include "malloc.h"
#include "portagree.h"
#include "download.h"

static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx0_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx1_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx2_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx3_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx4_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx5_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx6_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx7_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) dmaRx8_buffer[UART_DMA_RX_TX_CACHE_BUFFER_SIZE]; 

UartDeviceContext_t uartDevices[UART_DEV_NUM] = {0};

static UART_HandleTypeDef huart1,
													huart2,
													huart3,
													huart4,
													huart5,
													huart6,
													huart7,
													huart8,
													huart9;

static DMA_HandleTypeDef hdma_usart1_rx,hdma_usart2_rx,hdma_usart3_rx,
	                       hdma_usart4_rx,hdma_usart5_rx,hdma_usart6_rx,
                         hdma_usart7_rx,hdma_usart8_rx,hdma_usart9_rx;

static uint8_t *dmaRxBufer[9];

extern void Error_Handler(void);

UART_HandleTypeDef *get_uart_handle(uint8_t port)
{ 
   switch(port)
	 { 
	    case PORTA:/*0*/
		  return &huart4;
			
			case PORTB:/*1*/
			return &huart9;
			
			case PORTC:/*2*/
			return &huart2;
			
			case PORTD:/*3*/
			return &huart1;
			
			case PORTE:/*4*/
			return &huart5;
			
			case PORTF:/*5*/
			return &huart6;
			
			case PORTG:/*6*/
			return &huart7;
			
			case PORTH:/*7*/
			return &huart8;
			
			default:
			break;
	 }
	 return NULL;
}
void MX_USART1_UART_Init(uint32_t BaudRate)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = BaudRate;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);
}

void MX_USART2_UART_Init(uint32_t BaudRate)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = BaudRate;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);
}

void MX_USART3_UART_Init(uint32_t BaudRate)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = BaudRate;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  HAL_UART_Init(&huart3);
}


void MX_UART4_Init(uint32_t BaudRate)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = BaudRate;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart4);
}


void MX_UART5_Init(uint32_t BaudRate)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = BaudRate;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart5);

}

void MX_UART6_Init(uint32_t BaudRate)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = BaudRate;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart6);
}

void MX_UART7_Init(uint32_t BaudRate)
{

  huart7.Instance = UART7;
  huart7.Init.BaudRate = BaudRate;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart7);
}

void MX_UART8_Init(uint32_t BaudRate)
{

  huart8.Instance = UART8;
  huart8.Init.BaudRate = BaudRate;
  huart8.Init.WordLength = UART_WORDLENGTH_8B;
  huart8.Init.StopBits = UART_STOPBITS_1;
  huart8.Init.Parity = UART_PARITY_NONE;
  huart8.Init.Mode = UART_MODE_TX_RX;
  huart8.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart8.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart8);
}

void MX_UART9_Init(uint32_t BaudRate)
{

  huart9.Instance = UART9;
  huart9.Init.BaudRate = BaudRate;
  huart9.Init.WordLength = UART_WORDLENGTH_8B;
  huart9.Init.StopBits = UART_STOPBITS_1;
  huart9.Init.Parity = UART_PARITY_NONE;
  huart9.Init.Mode = UART_MODE_TX_RX;
  huart9.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart9.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart9);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	 GPIO_InitTypeDef GPIO_InitStruct = {0};
   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

   if(huart->Instance==USART1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16910CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


   /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream5;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart1_rx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);

    HAL_UART_Receive_DMA(huart,dmaRxBufer[3],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
  else if(huart->Instance==USART2)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA2_Stream6;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_USART2_RX;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[2],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
	else if(huart->Instance==USART3)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA2_Stream7;
    hdma_usart3_rx.Init.Request = DMA_REQUEST_USART3_RX;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart3_rx);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[8],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
	else if(huart->Instance==UART4)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PB8     ------> UART4_RX
    PB9     ------> UART4_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    #if 1
    /* USART4 DMA Init */
    /* USART4_RX Init */
    hdma_usart4_rx.Instance = DMA2_Stream0;
    hdma_usart4_rx.Init.Request = DMA_REQUEST_UART4_RX;
    hdma_usart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart4_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart4_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart4_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart4_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart4_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart4_rx);
    #endif
    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[0],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
   else if(huart->Instance==UART5)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART5;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**UART5 GPIO Configuration
    PB5     ------> UART5_RX
    PB6     ------> UART5_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_UART5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    hdma_usart5_rx.Instance = DMA2_Stream1;
    hdma_usart5_rx.Init.Request = DMA_REQUEST_UART5_RX;
    hdma_usart5_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart5_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart5_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart5_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart5_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart5_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart5_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart5_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart5_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart5_rx);


    HAL_NVIC_SetPriority(UART5_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);

    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[4],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
  else  if(huart->Instance==USART6)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16910CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA1_Stream3;
    hdma_usart6_rx.Init.Request = DMA_REQUEST_USART6_RX;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart6_rx);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[5],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
	 else if(huart->Instance==UART7)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART7;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
    /* Peripheral clock enable */
    __HAL_RCC_UART7_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**UART7 GPIO Configuration
    PB3(JTDO/TRACESWO)     ------> UART7_RX
    PB4(NJTRST)     ------> UART7_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_UART7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    #if 1
    /* USART7 DMA Init */
    /* USART7_RX Init */
    hdma_usart7_rx.Instance = DMA2_Stream2;
    hdma_usart7_rx.Init.Request = DMA_REQUEST_UART7_RX;
    hdma_usart7_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart7_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart7_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart7_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart7_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart7_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart7_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart7_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart7_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart7_rx);
    #endif
    /* UART7 interrupt Init */
    HAL_NVIC_SetPriority(UART7_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[6],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
	 else if(huart->Instance==UART8)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART8;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_UART8_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**UART8 GPIO Configuration
    PE0     ------> UART8_RX
    PE1     ------> UART8_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    #if 1
    /* USART8 DMA Init */
    /* USART8_RX Init */
    hdma_usart8_rx.Instance = DMA2_Stream3;
    hdma_usart8_rx.Init.Request = DMA_REQUEST_UART8_RX;
    hdma_usart8_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart8_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart8_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart8_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart8_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart8_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart8_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart8_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart8_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart8_rx);
    #endif
    /* UART8 interrupt Init */
    HAL_NVIC_SetPriority(UART8_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(UART8_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[7],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
  }
   else if(huart->Instance==UART9)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART9;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16910CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_UART9_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART9 GPIO Configuration
    PD14     ------> UART9_RX
    PD15     ------> UART9_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_UART9;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    #if 1
   /* USART9 DMA Init */
    /* USART9_RX Init */
    hdma_usart9_rx.Instance = DMA2_Stream4;
    hdma_usart9_rx.Init.Request = DMA_REQUEST_UART9_RX;
    hdma_usart9_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart9_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart9_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart9_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart9_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart9_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart9_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart9_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart9_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart9_rx);
    #endif
    /* UART9 interrupt Init */
    HAL_NVIC_SetPriority(UART9_IRQn,6, 0);
    HAL_NVIC_EnableIRQ(UART9_IRQn);

    //开启空闲中断
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(huart,UART_IT_ORE);
    HAL_UART_Receive_DMA(huart,dmaRxBufer[1],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);

  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	 if(huart->Instance==UART4)
  {
    __HAL_RCC_UART4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(UART4_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);

  }
  else if(huart->Instance==UART5)
  {
    __HAL_RCC_UART5_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(UART5_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==UART7)
  {
    __HAL_RCC_UART7_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(UART7_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==UART8)
  {
    __HAL_RCC_UART8_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_1);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(UART8_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==UART9)
  {
    __HAL_RCC_UART9_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_14|GPIO_PIN_15);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(UART9_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==USART2)
  {
    __HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(USART3_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
  else if(huart->Instance==USART6)
  {
    __HAL_RCC_USART6_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
    __HAL_UART_DISABLE_IT(huart,UART_IT_IDLE);
  }
}

void changerUsartBaudrate(uint8_t index,uint32_t Baudate)
{
	 UartDeviceContext_t *pDev = &uartDevices[index];
	 if(pDev->uartMutex == NULL)return;
	
	 if(xSemaphoreTake(pDev->uartMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
		   HAL_UART_Abort_IT(pDev->huart);
			// 确保UART处于就绪状态
        while(pDev->huart->gState != HAL_UART_STATE_READY) {
            vTaskDelay(1);
        }		 
				 pDev->huart->Init.BaudRate = Baudate;
			switch(index)
		{
     case 0:
		 HAL_DMA_Abort(&hdma_usart4_rx);
		 MX_UART4_Init(Baudate);
	 	 break;

	   case 1:
		 HAL_DMA_Abort(&hdma_usart9_rx);
		 MX_UART9_Init(Baudate);
	 	 break;

	   case 2:
		 HAL_DMA_Abort(&hdma_usart2_rx);
		 MX_USART2_UART_Init(Baudate);
	   break;

     case 3:
		 HAL_DMA_Abort(&hdma_usart1_rx);
		 MX_USART1_UART_Init(Baudate);
	 	 break;

	   case 4:
		 HAL_DMA_Abort(&hdma_usart5_rx);
		 MX_UART5_Init(Baudate);
	 	 break;

	   case 5:
		 HAL_DMA_Abort(&hdma_usart6_rx);
	   MX_UART6_Init(Baudate);
	 	 break;

	   case 6:
		 HAL_DMA_Abort(&hdma_usart7_rx);
		 MX_UART7_Init(Baudate);
	 	 break;

	   case 7:
		 HAL_DMA_Abort(&hdma_usart8_rx);
		 MX_UART8_Init(Baudate);
	 	 break;

	   case 8:
		 HAL_DMA_Abort(&hdma_usart3_rx);
		 MX_USART3_UART_Init(Baudate);
	 	 break;
		}				
		 xSemaphoreGive(pDev->uartMutex);
	}
}
 
BaseType_t MultiUart_SendMoreByte(UartDevice_t devId, uint8_t *data, uint16_t len, uint32_t timeout, uint8_t priority)
{ 
		if(devId >= UART_DEV_NUM) return pdFALSE;
    
    UartDeviceContext_t *pDev = &uartDevices[devId];
    DEV_UartFrame_t frame; 
	  memset((DEV_UartFrame_t*)&frame,0,sizeof(DEV_UartFrame_t));
	
	  memcpy(frame.pData,data,len);
	
    frame.len = len;
    frame.timeout = timeout;
    frame.priority = priority;
   
    // 根据优先级选择入队方式
    BaseType_t result;
    if(priority > 200) {
			  result = xQueueSendToFront(pDev->devTxQueue, &frame, pdMS_TO_TICKS(timeout));
    } else {
			  result = xQueueSend(pDev->devTxQueue, &frame, pdMS_TO_TICKS(timeout));
    }   
    // 发送失败时释放内存
    if(result != pdPASS) {
			 ;
    }
    
    return result;	
}

BaseType_t MultiUart_SendMoreByteISR(UartDevice_t devId, uint8_t *data, uint16_t len, uint32_t timeout, uint8_t priority,BaseType_t *pxHigherPriorityTaskWoken)
{ 
		if(devId >= UART_DEV_NUM) return pdFALSE;
    
    UartDeviceContext_t *pDev = &uartDevices[devId]; 
    DEV_UartFrame_t frame; 
    memset((DEV_UartFrame_t*)&frame,0,sizeof(DEV_UartFrame_t));	
	  memcpy(frame.pData,data,len);
	
    frame.len = len;
    frame.timeout = timeout;
    frame.priority = priority;
   
    // 根据优先级选择入队方式
    BaseType_t result;
    // 使用中断安全的队列操作函数
    if(priority > 200) {
        // 紧急帧：插入队列头部（中断安全版本）
       // result = xQueueSendToFrontFromISR(pDev->txQueue, &frame, pxHigherPriorityTaskWoken);
			    result = xQueueSendToFrontFromISR(pDev->devTxQueue, &frame, pxHigherPriorityTaskWoken);
    } else {
        // 普通帧：插入队列尾部（中断安全版本）
        result = xQueueSendFromISR(pDev->devTxQueue, &frame, pxHigherPriorityTaskWoken);
    }
    
    // 发送失败时释放内存
    if(result != pdPASS) {
       // myfree(SRAMIN,frame.pData);
			//	vPortFree(frame.pData);
			//  frame.pData = NULL;
			 ;
    }
    
    return result;	
}
// 发送帧数据到指定串口
BaseType_t MultiUart_SendFrame(UartDevice_t devId,
															 uint8_t *data,
															 uint16_t len,
															 uint8_t type,
															 uint8_t index, 
                               uint32_t timeout,
														   uint8_t priority)
{
    if(devId >= UART_DEV_NUM) return pdFALSE;
    
    UartDeviceContext_t *pDev = &uartDevices[devId];
	  DEV_UartFrame_t frame;
	
    memset((DEV_UartFrame_t*)&frame,0,sizeof(DEV_UartFrame_t));	
	  memcpy(frame.pData,data,len);
	
    frame.len = len+7;
    frame.timeout = timeout;
    frame.priority = priority;
    
    memset(frame.pData,0,len + 7);
	
	  frame.pData[0] = 0x5A;
	  frame.pData[1] = 0x97;
	  frame.pData[2] = type;
	  frame.pData[3] = len;
	  frame.pData[4] = index;
	
	  for(uint16_t i = 0;i<len;i++)
	 { 
	   frame.pData[5+i] = data[i];
	 }
	 
	 uint32_t checksum = 0;
	 for (size_t i = 0; i < frame.len - 2; i++) {
		checksum +=frame.pData[i];
   }	
	 frame.pData[len + 5] = checksum&0xFF;
	 frame.pData[len + 6] = 0xA5;	
	
    // 根据优先级选择入队方式
    BaseType_t result;
    if(priority > 200) {
        // 紧急帧：插入队列头部
        result = xQueueSendToFront(pDev->devTxQueue, &frame, pdMS_TO_TICKS(10));
    } else {
        // 普通帧：插入队列尾部
        result = xQueueSend(pDev->devTxQueue, &frame, pdMS_TO_TICKS(10));
    }
    
    // 发送失败时释放内存
    if(result != pdPASS) {
        //myfree(SRAMIN,frame.pData);
			 // vPortFree(frame.pData);
			//  frame.pData = NULL;
			;
    }
    
    return result;
}
BaseType_t MultiUart_SendFrame_FromISR(UartDevice_t devId,
																			 uint8_t *data,
																		   uint16_t len,
																			 uint8_t type,
																	     uint8_t index,
																		   uint32_t timeout,
																			 uint8_t priority,
																			 BaseType_t *pxHigherPriorityTaskWoken)
{
    if(devId >= UART_DEV_NUM) return pdFALSE;
    
    UartDeviceContext_t *pDev = &uartDevices[devId];
    BaseType_t xResult;
    DEV_UartFrame_t frame;
    memset((DEV_UartFrame_t*)&frame,0,sizeof(DEV_UartFrame_t));	
	  memcpy(frame.pData,data,len);
	
    // 构建帧数据
    frame.len = len + 7;
    frame.timeout = timeout;
    frame.priority = priority;
    
    // 构建帧（与原始版本相同）
    frame.pData[0] = 0x5A;
    frame.pData[1] = 0x97;
    frame.pData[2] = type;
    frame.pData[3] = len;
    frame.pData[4] = index;
    
    // 复制数据
    for(uint16_t i = 0; i < len; i++) {
        frame.pData[5 + i] = data[i];
    }
    
    // 计算校验和
    uint32_t checksum = 0;
    for (size_t i = 0; i < frame.len - 2; i++) {
        checksum += frame.pData[i];
    }
    frame.pData[len + 5] = checksum & 0xFF;
    frame.pData[len + 6] = 0xA5;
    
    // 使用中断安全的队列操作函数
    if(priority > 200) {
        // 紧急帧：插入队列头部（中断安全版本）
        xResult = xQueueSendToFrontFromISR(pDev->devTxQueue, &frame, pxHigherPriorityTaskWoken);
    } else {
        // 普通帧：插入队列尾部（中断安全版本）
        xResult = xQueueSendFromISR(pDev->devTxQueue, &frame, pxHigherPriorityTaskWoken);
    }
 
    return xResult;
}
 
void USART1_IRQHandler(void)
{
     
    if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);  // 清除中断标记      
        HAL_UART_DMAStop(&huart1);
				uint16_t length = (UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx));			
   			  if(length !=0)
				{
				   SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[PORTD], length);			 
					 scan_agreement_data_port_dev(PORTD,dmaRxBufer[PORTD],length);
				}
        memset(dmaRxBufer[PORTD],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart1,dmaRxBufer[PORTD],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart1);
}

void USART2_IRQHandler(void)/*B*/
{
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)  
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);  
        HAL_UART_DMAStop(&huart2);
			  uint16_t length = (UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx));
   			  if(length !=0)
				{
					SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[PORTC], length);
					scan_agreement_data_port_dev(PORTC,dmaRxBufer[PORTC],length);
				}
        memset(dmaRxBufer[PORTC],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart2,dmaRxBufer[PORTC],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart2);
}

void USART3_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);  // 清除中断标记
        HAL_UART_DMAStop(&huart3);
			  uint16_t length = (UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx));
        if(length!=0)
				{
				   SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[BLUE],length);				  
           scan_agreement_data_port_dev(BLUE,dmaRxBufer[BLUE],length); 
				}
				memset(dmaRxBufer[BLUE],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart3,dmaRxBufer[BLUE],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart3);
}

void UART4_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart4, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart4);  // 清除中断标记
        HAL_UART_DMAStop(&huart4);
			  uint16_t length= (UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart4.hdmarx));
        if(length!=0)
				{
			  	  SCB_InvalidateDCache_by_Addr ((uint32_t *)dmaRxBufer[PORTA], length);
					  scan_agreement_data_port_dev(PORTA,dmaRxBufer[PORTA],length);
				}
				memset(dmaRxBufer[PORTA],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart4,dmaRxBufer[PORTA],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart4);
}

void UART5_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart5, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart5);  // 清除中断标记
        HAL_UART_DMAStop(&huart5);
			  uint16_t length=(UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart5.hdmarx));
			  if(length !=0)
				{
				  SCB_InvalidateDCache_by_Addr ((uint32_t *)dmaRxBufer[PORTE],length);
					scan_agreement_data_port_dev(PORTE,dmaRxBufer[PORTE],length);
				}
				memset(dmaRxBufer[PORTE],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart5,dmaRxBufer[PORTE],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart5);
}

void USART6_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart6, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart6);  // 清除中断标记
        HAL_UART_DMAStop(&huart6);
			  uint16_t length=(UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart6.hdmarx));
      	 if(length!=0)
				{
					SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[PORTF],length);
					scan_agreement_data_port_dev(PORTF,dmaRxBufer[PORTF],length);
				}
				memset(dmaRxBufer[PORTF],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart6,dmaRxBufer[PORTF],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart6);
}

void UART7_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart7, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart7);  // 清除中断标记
        HAL_UART_DMAStop(&huart7);
			  uint16_t length=(UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart7.hdmarx));
     		if(length !=0)
				{
					SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[PORTG],length);
					scan_agreement_data_port_dev(PORTG,dmaRxBufer[PORTG],length);
				}
				memset(dmaRxBufer[PORTG],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart7,dmaRxBufer[PORTG],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart7);
}

void UART8_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart8, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart8);  // 清除中断标记
        HAL_UART_DMAStop(&huart8);
			  uint16_t length=(UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart8.hdmarx));
        if(length !=0)
				{
					   SCB_InvalidateDCache_by_Addr ((uint32_t *)dmaRxBufer[PORTH], length);	
             scan_agreement_data_port_dev(PORTH,dmaRxBufer[PORTH],length);					
				}
				memset(dmaRxBufer[PORTH],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
        HAL_UART_Receive_DMA(&huart8,dmaRxBufer[PORTH],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart8);
}
 
void UART9_IRQHandler(void)
{
	  uint16_t length;
    if(__HAL_UART_GET_FLAG(&huart9, UART_FLAG_IDLE) != RESET)  // 空闲中断标记被置位
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart9);  // 清除中断标记
        HAL_UART_DMAStop(&huart9);
			  length=(UART_DMA_RX_TX_CACHE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart9.hdmarx));
        if(length!=0)
		    {  
					 SCB_InvalidateDCache_by_Addr((uint32_t *)dmaRxBufer[PORTB],length);
					 scan_agreement_data_port_dev(PORTB,dmaRxBufer[PORTB],length);
				}
				memset(dmaRxBufer[PORTB],0,UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
       HAL_UART_Receive_DMA(&huart9,dmaRxBufer[PORTB],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
    }
      HAL_UART_IRQHandler(&huart9);
}

static void ClearUARTErrors(UART_HandleTypeDef *huart)
{
   // 清除错误标志
   __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF |UART_CLEAR_IDLEF
                                |UART_CLEAR_TCF|UART_CLEAR_LBDF|UART_CLEAR_CTSF|UART_CLEAR_CMF|UART_CLEAR_WUF|UART_CLEAR_TXFECF);

   __HAL_UART_CLEAR_OREFLAG(huart);
	 __HAL_UART_CLEAR_IDLEFLAG(huart);
   huart->RxState = HAL_UART_STATE_READY;
   huart->Lock = HAL_UNLOCKED;
}

 
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	 if(huart->Instance == USART1)
	 {
	    ClearUARTErrors(&huart1);
	    HAL_UART_Receive_DMA(&huart1,dmaRxBufer[PORTD],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == USART2)
	 {
		 ClearUARTErrors(&huart2);
	   HAL_UART_Receive_DMA(&huart2,dmaRxBufer[PORTC],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == USART3)
	 {
		 ClearUARTErrors(&huart3);
	   HAL_UART_Receive_DMA(&huart3,dmaRxBufer[BLUE],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == UART4)
	 {
		 ClearUARTErrors(&huart4);
	   HAL_UART_Receive_DMA(&huart4,dmaRxBufer[PORTA],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == UART5)
	 {
		 ClearUARTErrors(&huart5);
	   HAL_UART_Receive_DMA(&huart5,dmaRxBufer[PORTE],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
   else if(huart->Instance == USART6)
	 {
	    ClearUARTErrors(&huart6);
		  HAL_UART_Receive_DMA(&huart6,dmaRxBufer[PORTF],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == UART7)
	 {
		 ClearUARTErrors(&huart7);
	   HAL_UART_Receive_DMA(&huart7,dmaRxBufer[PORTG],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == UART8)
	 {
		  ClearUARTErrors(&huart8);
	    HAL_UART_Receive_DMA(&huart8,dmaRxBufer[PORTH],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }
	 else if(huart->Instance == UART9)
	 {
	   ClearUARTErrors(&huart9);
	   HAL_UART_Receive_DMA(&huart9,dmaRxBufer[PORTB],UART_DMA_RX_TX_CACHE_BUFFER_SIZE);
	 }

}

void bsp_DeInit_usart(void)
{
HAL_UART_MspDeInit(&huart1);
HAL_UART_MspDeInit(&huart2);
HAL_UART_MspDeInit(&huart3);
HAL_UART_MspDeInit(&huart4);
HAL_UART_MspDeInit(&huart5);
HAL_UART_MspDeInit(&huart6);
HAL_UART_MspDeInit(&huart7);
HAL_UART_MspDeInit(&huart8);
HAL_UART_MspDeInit(&huart9);
}

void bsp_init_usart(void)
{
	// for(uint8_t i = 0;i<9;i++)
	 //{
	   // dmaRxBufer[i] = (uint8_t *)mymalloc(SRAM4,256);
		  //if(dmaRxBufer[i] == NULL)
			//{ /*memory malloc error*/while(1);}
	 //}
	dmaRxBufer[0] = dmaRx0_buffer;
	dmaRxBufer[1] = dmaRx1_buffer;
	dmaRxBufer[2] = dmaRx2_buffer;
	dmaRxBufer[3] = dmaRx3_buffer;
	dmaRxBufer[4] = dmaRx4_buffer;
	dmaRxBufer[5] = dmaRx5_buffer;
	dmaRxBufer[6] = dmaRx6_buffer;
	dmaRxBufer[7] = dmaRx7_buffer;
	dmaRxBufer[8] = dmaRx8_buffer;

    /*3.DeInit Usart*/
	  bsp_DeInit_usart();

 		/*4.Init usart*/
	  __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

    MX_USART1_UART_Init(115200);
	  MX_USART2_UART_Init(115200);
	  MX_USART3_UART_Init(115200);
	  MX_UART4_Init(115200);
	  MX_UART5_Init(115200);
	  MX_UART6_Init(115200);
	  MX_UART7_Init(115200);
	  MX_UART8_Init(115200);
	  MX_UART9_Init(115200);
}

void MultiUart_Init(void)
{ 
	  extern void vUartSendTask(void *pvParameters);
	  extern void vDevUartSendTask(void *pvParameters);
	  extern void vDevControlTask(void *pvParameters);
	  extern void xDevEventDisposeTask(void *pvParameters);
	  
	  UART_HandleTypeDef *huart[8] = {&huart4,&huart9,&huart2,&huart1,&huart5,&huart6, &huart7,&huart8};
	  for(uint8_t i = PORTA;i<(UART_DEV_NUM - 2);i++)
	 {
    uartDevices[i].huart = huart[i];
	  uartDevices[i].txCompleteSem = xSemaphoreCreateBinary();
	  uartDevices[i].uartMutex = xSemaphoreCreateMutex();
		if(uartDevices[i].uartMutex == NULL || uartDevices[i].txCompleteSem == NULL){while(1);}
		uartDevices[i].devTxQueue = xQueueCreate(1, sizeof(DEV_UartFrame_t));
		uartDevices[i].devControlQueue = xQueueCreate(32, sizeof(void *));
    xTaskCreate(vDevUartSendTask, "DEV_PORT_TX", 256, (void*)i, configMAX_PRIORITIES - 1, &uartDevices[i].devTxHandle);
		xTaskCreate(vDevControlTask,  "DEV_ControlTask", 256, (void*)i, configMAX_PRIORITIES - 2, &uartDevices[i].devControlTaskHandle);	    
	 }

    // 初始化串口BLUE
    uartDevices[BLUE].huart = &huart3;
    uartDevices[BLUE].txCompleteSem = xSemaphoreCreateBinary();
		uartDevices[BLUE].uartMutex = xSemaphoreCreateMutex();		
		if(uartDevices[BLUE].uartMutex == NULL || uartDevices[BLUE].txCompleteSem == NULL){while(1);}
		uartDevices[BLUE].devTxQueue = xQueueCreate(1, sizeof(DEV_UartFrame_t));
		xTaskCreate(vDevUartSendTask, "BLUE_TX", 256, (void*)BLUE, configMAX_PRIORITIES - 2, &uartDevices[BLUE].devTxHandle);
		
    // 初始化串口USB
    uartDevices[USB].huart = NULL;
	  uartDevices[USB].devTxQueue = xQueueCreate(1, sizeof(DEV_UartFrame_t));
		xTaskCreate(vDevUartSendTask, "USB_TX", 256, (void*)USB,  configMAX_PRIORITIES - 2, &uartDevices[USB].devTxHandle); 	
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    
    for (int i = 0; i < UART_DEV_NUM; i++) {
        if (uartDevices[i].huart == huart) {
            // 给出发送完成的信号量
					   if(huart->gState == HAL_UART_STATE_READY) {
							 xSemaphoreGiveFromISR(uartDevices[i].txCompleteSem, NULL);
						 }           
            break;
        }
    }
}

void DMA2_Stream0_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart4_rx);}
void DMA2_Stream1_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart5_rx);}
void DMA2_Stream2_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart7_rx);}
void DMA2_Stream3_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart8_rx);}
void DMA2_Stream4_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart9_rx);}
void DMA2_Stream5_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart1_rx);}
void DMA2_Stream6_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart2_rx);}
void DMA2_Stream7_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart3_rx);}
void DMA1_Stream3_IRQHandler(void){HAL_DMA_IRQHandler(&hdma_usart6_rx);}


