/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* IntelliSense fallback: keep core HAL macros available even when
 * language server compile flags are incomplete. */
#ifndef USE_HAL_DRIVER
#define USE_HAL_DRIVER
#endif

#ifndef STM32H723xx
#define STM32H723xx
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
//#include "malloc.h"
#include "sys.h"
#include "delay.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "dac.h"
#include "gpio.h"
#include "iic.h"
#include "iwdg.h"
#include "ospi.h"
#include "spi.h"
#include "timer.h"
#include "uart.h"
#include "mem.h"	
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#include "w25qxx.h"
#include "key.h"
#include "matrix.h"
#include "music.h"
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#include "FreeRTOS.h"
#include "task.h"
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#include "devfile.h"
#include "rawMatrix.h"
#include "matchineState.h"
#include "portagree.h"
#include "download.h"
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
void iwdg_init(uint32_t prer, uint16_t rlr);
void iwdg_feed(void);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
