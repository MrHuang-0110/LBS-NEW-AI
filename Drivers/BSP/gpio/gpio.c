#include "main.h"

EXTI_HandleTypeDef MemInt;
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, I2C_SDA_IO_Pin|I2C_SCL_IO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SystemLED_IO_GPIO_Port, SystemLED_IO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, SystemPower_IO_Pin|BlueStateLED_IO_Pin, GPIO_PIN_RESET);

  /* ≈‰÷√PA0Œ™ ‰»Î */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  #if IIC_SALVE_MODE == 0
  /*Configure GPIO pins : I2C_SDA_IO_Pin I2C_SCL_IO_Pin */
  GPIO_InitStruct.Pin =   I2C_SCL_IO_Pin;
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull =  GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  GPIO_InitStruct.Pin =   I2C_SDA_IO_Pin;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  #endif

  /*Configure GPIO pin : SoundPower_IO_Pin */
  GPIO_InitStruct.Pin = SoundPower_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SoundPower_IO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BlueState_IO_Pin MiddKey_IO_Pin LeftKey_IO_Pin RightKey_IO_Pin */
  GPIO_InitStruct.Pin = BlueState_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =  MiddKey_IO_Pin|LeftKey_IO_Pin|RightKey_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
 
  /*Configure GPIO pin : SystemLED_IO_Pin */
  GPIO_InitStruct.Pin = SystemLED_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SystemLED_IO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SystemPower_IO_Pin BlueStateLED_IO_Pin */
  GPIO_InitStruct.Pin = SystemPower_IO_Pin|BlueStateLED_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);

	  /* EXTI interrupt init*/
    //HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    //HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void bsp_InitGpio(void)
{
    MX_GPIO_Init();
}
