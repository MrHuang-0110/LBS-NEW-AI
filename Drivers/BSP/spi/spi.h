#ifndef __BSP_SPI_H
#define __BSP_SPI_H


void bsp_InitSpi(void);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi);
SPI_HandleTypeDef *get_hspi_handle(void);
 
#endif
