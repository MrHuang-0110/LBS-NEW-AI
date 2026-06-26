
#ifndef _BSP_USART_H_
#define _BSP_USART_H_

#include "sys.h"


#define UART_DMA_RX_TX_CACHE_BUFFER_SIZE 300
 
typedef enum
{ 
   PORTA,
	 PORTB,
	 PORTC, 
	 PORTD,
	 PORTE,
	 PORTF,
	 PORTG,
	 PORTH,	
	 BLUE,
	 USB,
	 UART_DEV_NUM
}UartDevice_t;

 #if 0
typedef struct {
    uint8_t  *pData;       
    uint16_t len;         
    uint32_t timeout;     
    uint8_t priority;    
} UartFrame_t;
#endif

typedef struct {
    uint8_t  pData[UART_DMA_RX_TX_CACHE_BUFFER_SIZE];       
    uint16_t len;         
    uint32_t timeout;     
    uint8_t priority;    
}DEV_UartFrame_t;


typedef struct {
    UART_HandleTypeDef *huart;    
    QueueHandle_t devTxQueue,devControlQueue;        
    TaskHandle_t devTxHandle,devControlTaskHandle; 
	  //DEV_UartFrame_t frame;
    uint32_t txCount;              
    uint32_t errorCount; 
    SemaphoreHandle_t txCompleteSem;	
	  SemaphoreHandle_t  uartMutex;
} UartDeviceContext_t;

 

extern UartDeviceContext_t uartDevices[UART_DEV_NUM];

UART_HandleTypeDef *get_uart_handle(uint8_t port);
void changerUsartBaudrate(uint8_t index,uint32_t Baudate);
void MultiUart_Init(void);
void bsp_init_usart(void);

BaseType_t MultiUart_SendMoreByte(UartDevice_t devId, uint8_t *data, uint16_t len, uint32_t timeout, uint8_t priority);
BaseType_t MultiUart_SendMoreByteISR(UartDevice_t devId, uint8_t *data, uint16_t len, uint32_t timeout, uint8_t priority,BaseType_t *pxHigherPriorityTaskWoken);
BaseType_t MultiUart_SendFrame(UartDevice_t devId,
															 uint8_t *data,
															 uint16_t len,
															 uint8_t type,
															 uint8_t index, 
                               uint32_t timeout,
														   uint8_t priority);
BaseType_t MultiUart_SendFrame_FromISR(UartDevice_t devId, uint8_t *data, uint16_t len,uint8_t type,uint8_t index, uint32_t timeout, uint8_t priority,BaseType_t *pxHigherPriorityTaskWoken);

#if 0
BaseType_t MultiUart_SendFrame(UartDevice_t devId, uint8_t *data, uint16_t len,uint8_t index, uint32_t timeout, uint8_t priority);
 
BaseType_t Uart_SendFrame(UartDevice_t devId, uint8_t *data, uint16_t len,uint8_t index);
BaseType_t MultiUart_SendFrame_FromISR(UartDevice_t devId, uint8_t *data, uint16_t len,
                                       uint8_t index, uint32_t timeout, uint8_t priority,
                                       BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t sensord_send_frame(UartDevice_t devId,uint8_t mode,uint8_t sensord_id,uint8_t *data,uint16_t len);
#endif
#endif
