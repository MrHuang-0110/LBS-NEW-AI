#include "blue.h"
#include "uart.h"
//#include "malloc.h"
static BLUE_CONFIG blueStruct;
static char __attribute__((section(".DTCM_Data"))) monitor_blue_buffer[2*1024]; 
 
 
BLUE_CONFIG *blueHandle(void)
{ 
   return &blueStruct;
}

static void blueScanATcmd(char *atcmd)
{ 
	 blueStruct.atState = 0;
	
   memset(blueStruct.atData,0,sizeof(blueStruct.atData));
	  
	 uint32_t start = xTaskGetTickCount();
	 uint32_t duration = 0;
	
	 while(blueStruct.atState!=1)
	 { 
     duration = xTaskGetTickCount() - start;
		 if(duration < 1000)
		 { 
		   if(duration%500 == 0)
				 MultiUart_SendMoreByte(BLUE,(unsigned char*)atcmd,strlen(atcmd),200,250);
		 }
		 else 
				 break;		
     
     vTaskDelay(10);		 
	 }
}
void close_blueAndFeedIWDG(void)
{ 
  blueStruct.atState = 0;
	
   memset(blueStruct.atData,0,sizeof(blueStruct.atData));
	  
	 uint32_t start = xTaskGetTickCount();
	 uint32_t duration = 0;
	
	 while(blueStruct.atState!=1)
	 { 
     duration = xTaskGetTickCount() - start;
		 if(duration < 1000)
		 { 
		   if(duration%500 == 0)
				 MultiUart_SendMoreByte(BLUE,(unsigned char*)"AT+ROLE=1\r\n",strlen("AT+ROLE=1\r\n"),200,250);
		 }
		 else 
				 break;		
     
     vTaskDelay(10);		 
		 extern void iwdg_feed(void);
     iwdg_feed();
	 }  
}
// 判断是否为有效的AT指令
bool is_valid_at_command(const char *data, size_t len) {
	
    if(data[len-1] == '\r' || data[len-1] == '\n')
					return true;
		
		return false;
}

void is_get_at_data(uint8_t *atData,uint8_t *pData,size_t len)
{ 
		while(len > 0 && (pData[len-1] == '\r' || pData[len-1] == '\n')) {
         len--;
    } 
		
		memcpy(atData,pData,len);
}

 

void blue_onAndoff(uint8_t on_off)
{ 
   if(on_off!=0)
	   blueScanATcmd("AT+ROLE=2\r\n");
	 else
		 blueScanATcmd("AT+ROLE=1\r\n");
}
 
void blue_init(void)
{ 
 /*1.设置蓝牙恢复出厂设置*/
	// blueScanATcmd("AT+FACTORY\r\n");
 
	 /*2.设置蓝牙名字*/
	 blueScanATcmd("AT+NAME=EST_BLUE\r\n");

	 /*3.设置蓝牙为从机*/
	 blueScanATcmd("AT+ROLE=2\r\n");

	 /*4.设置蓝牙Mode传输模式*/
	 blueScanATcmd("AT+MODE=1\r\n");

	 /*5.设置波特率为115200*/
	 blueScanATcmd("AT+UART=4\r\n");

	 /*6.设置蓝牙功率*/
	 blueScanATcmd("AT+POWE=9\r\n");

	 /*7.设置广播间隙*/
	 blueScanATcmd("AT+ADVINT=3\r\n");
	
	 /*获取蓝牙mac地址*/
   blueScanATcmd("AT+MAC?\r\n");
		
	 /*8.复位*/
	 blueScanATcmd("AT+RST\r\n");
	 
	 blueScanATcmd("AT+ROLE=1\r\n");
	 
	 blueStruct.atState = 0;
	 if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_10) == 1)
   {
		   vTaskDelay(100);
		   if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_10) == 1)
							blueStruct.blueState = 1;		  		 
   }
	 else
	 {
		  if(blueStruct.blueState == 1)
			{ 
					blue_onAndoff(1);
			}
			else
			{ 
				blue_onAndoff(0);
			  blueStruct.blueState = 0;
			}
	     
	 } 
	 
     blueStruct.blueMonitorBuffer = monitor_blue_buffer;
	   memset(blueStruct.blueMonitorBuffer,0,10*1024);
}
