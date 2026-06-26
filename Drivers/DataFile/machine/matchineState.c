#include "matchineState.h"
#include "rawMatrix.h"
#include "key.h"
#include "devfile.h"
#include "adc.h"
#include "portagree.h"
#include "download.h"
#include "uart.h"
#include "matrix.h"
#include "entery.h"
#include "blue.h"
#include "mem.h"
#include "remote.h"
#include "camer.h"
#include "motor.h"
#include "gray.h"
#include "touch.h"
#include "color.h"
#include "grayv2.h"
#include "nfc_car.h"

static uint8_t bat_level;

extern volatile uint8_t usb_cont_state;

float voic; 
float bat_percent; 

volatile bool is_monitor = true;
volatile bool BUS_LINE = false;
volatile bool newAiRunPython = false;
volatile bool monitor_state = true;
volatile bool USB_IDLE_TIME_FLAG = false,BLUE_IDLE_TIME_FLAG = false;

TimerHandle_t usbTimer = NULL,blueTimer = NULL;
TimerHandle_t xIWDGTimer = NULL;
TimerHandle_t remoteTimer = NULL;
TimerHandle_t xMonitorSendTimer = NULL;
TimerHandle_t xPortScanTimer = NULL; 

static volatile bool refreshMatrix = false;
static TimerHandle_t xKeyScanTimer = NULL;
static TimerHandle_t xBatSendTimer = NULL;
static TimerHandle_t xBlueSendTimer = NULL;
static TimerHandle_t xMemTimer = NULL;
 

static EventGroupHandle_t xEventGroup;
static volatile bool usbDownloadActive = false;
 
static SemaphoreHandle_t xUsbMutex = NULL;
static SemaphoreHandle_t xRunPythonMutex = NULL;
static SemaphoreHandle_t xRefreshMutex = NULL;

static TaskHandle_t SystemStartTaskHandle;
 
void MatChineStateTask(void *parament);
 
static void KeyScanTimerCallback(TimerHandle_t xTimer);
static void PortScanTimerCallback(TimerHandle_t xTimer);
static void MonitorSendTimerCallback(TimerHandle_t xTimer);
static void BatSendTimerCallback(TimerHandle_t xTimer);
static void BlueSendTimerCallback(TimerHandle_t xTimer);
static void USB_IdleTimeoutCallback(TimerHandle_t xTimer);
static void BLUE_IdleTimeoutCallback(TimerHandle_t xTimer);
static void IWDG_FreeTimeoutCallback(TimerHandle_t xTimer);
static void REMOTE_TimeoutCallback(TimerHandle_t xTimer);
static void MemUpdata_TimeoutCallback(TimerHandle_t xTimer);

uint8_t ucHeapAXI[256*1024];         
uint8_t __attribute__((section(".DTCM_Data")))ucHeapDCM[64*1024];
const HeapRegion_t xHeapRegions[] =
{
    { (uint8_t *)&ucHeapDCM,  64*1024 },
    { (uint8_t *)&ucHeapAXI,  256*1024},
    { NULL,                   0}    // ����������
};

 
void freeRtosHeapMemInit(void)
{ 
  vPortDefineHeapRegions(xHeapRegions);
}
void SET_EVENT_GROUP(EventBits_t event)
{ 
  xEventGroupSetBits(xEventGroup, event);
}
void SET_EVENT_GROUP_ISR(EventBits_t event)
{ 
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(
        xEventGroup, 
        event, 
        &xHigherPriorityTaskWoken
    );
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);  
}
bool getRunState(void)
{ 
   return newAiRunPython;
}
void NEWAI_CreatePowerOnStartTask(void)
{
     BaseType_t xReturn = pdPASS;
     taskENTER_CRITICAL();
	
     xReturn = xTaskCreate((TaskFunction_t	)MatChineStateTask,
				                   (const char* 	  )"MatChineState",
				                   (uint32_t 		    )2*1024,
				                   (void* 		  	  )NULL,
				                   (UBaseType_t 	  )TASK_PRIO_MAT_CHINE_STATE,
				                   (TaskHandle_t *  )SystemStartTaskHandle);

     if(xReturn!=pdPASS)
     {
         while(1);
     }
		 
     xUsbMutex = xSemaphoreCreateMutex();
		 xRunPythonMutex = xSemaphoreCreateMutex();
		 xRefreshMutex = xSemaphoreCreateMutex();
		 extern SemaphoreHandle_t xFsMutex;
		 xFsMutex = xSemaphoreCreateMutex();
		 
		 MultiUart_Init();

		 xEventGroup = xEventGroupCreate();
		    	
     taskEXIT_CRITICAL();
     vTaskStartScheduler();
}

void vDevControlTask(void *pvParameters)
{ 
    UartDevice_t devId = (UartDevice_t)(uintptr_t)pvParameters;
    UartDeviceContext_t *pDev = &uartDevices[devId]; 
	  TickType_t start_ticks = xTaskGetTickCount();
    void *self;
    while(1)
    { 
           /*Teset Write txt*/
		   if(xQueueReceive(pDev->devControlQueue, &self, portMAX_DELAY) == pdPASS) {
				    SensorBase *base = (SensorBase*)self;
				    if(base!=NULL)
						{
						   switch(base->type)
							 {
							   case DEV_ID_BIG_MOTOR:
								 case DEV_ID_SMALL_Motor:
											refsh_motor((DEV_MOTOR*)base,base->data);
								 break;
										
								 case DEV_ID_GRAY:
											refsh_gray((DEV_GRAY*)base,base->data);
								 break;
								 
								 case DEV_ID_TOUCH:
									 refsh_touch((DEV_TOUCH*)base,base->data);
								 break;
								 
								 case DEV_ID_ULTRASION:
									 refsh_ultrasion((DEV_ULTRASION*)base,base->data);
								 break;  
								 
								 case DEV_ID_COLOR:
									 refsh_color((DEV_COLOR*)base,base->data);
								 break;
								 
								 case DEV_ID_CAMER:
									 refsh_camer((DEV_CAMER*)base,base->findIndex,base->data);
								 break;
								 
								 case DEV_ID_GRAY_V2:
									 refsh_gray_v2((DEV_GRAYV2*)base,base->data);
									  
								 break;
								 
								 case DEV_ID_NFC:
									 refsh_nfc((DEV_NFC*)base,base->data);
								 break;
							 }
						}
			 }
		//	 taskYIELD();
		}			
}
void vDevUartSendTask(void *pvParameters)
{
    UartDevice_t devId = (UartDevice_t)(uintptr_t)pvParameters;
    UartDeviceContext_t *pDev = &uartDevices[devId];
    DEV_UartFrame_t frame;  
    
    while(1) {
        if(xQueueReceive(pDev->devTxQueue, &frame, portMAX_DELAY) == pdPASS) {
            HAL_StatusTypeDef status;				  
					  if(pDev->huart != NULL) {
					   if(pDev->uartMutex != NULL)
							{
									if(xSemaphoreTake(pDev->uartMutex,  pdMS_TO_TICKS(1000)) == pdTRUE) {
									status = HAL_UART_Transmit_IT(pDev->huart, frame.pData, frame.len);
									if(status == HAL_OK) {
										if(xSemaphoreTake(pDev->txCompleteSem, pdMS_TO_TICKS(1000)) != pdTRUE) {
                       HAL_UART_Abort_IT(pDev->huart);
                       status = HAL_TIMEOUT;
                    }
                  } else {
                        ;
                }		
                xSemaphoreGive(pDev->uartMutex);			
								}
							}
						}
						else
						{ 
							  extern SemaphoreHandle_t usbTxCompleteSem;
                extern uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len);
							  if(usbTxCompleteSem == NULL) {
                    usbTxCompleteSem = xSemaphoreCreateBinary();
									  xSemaphoreTake(usbTxCompleteSem, 0);
                }
								
								if(CDC_Transmit_HS(frame.pData, frame.len) == 0) {
                    if(xSemaphoreTake(usbTxCompleteSem, pdMS_TO_TICKS(1000)) == pdTRUE) {
                        status = HAL_OK;
                    } else {
                        status = HAL_TIMEOUT;
                    }
                }											  
						}
            if(status == HAL_OK) {
                pDev->txCount++;
            } else {
                pDev->errorCount++;
            }
        }
			 	taskYIELD();
    }
}

void EnteryTask(void *pvParameters)
{ 
     while(1) {      
        EventBits_t uxEvents = xEventGroupWaitBits(
            xEventGroup,
            EVENT_RUN_PYTHON,
            pdTRUE,       
            pdFALSE,       
            portMAX_DELAY
        );    
				 if(uxEvents & EVENT_RUN_PYTHON){ 
				   if(xSemaphoreTake(xRunPythonMutex, portMAX_DELAY) == pdPASS) {
						     newAiRunPython = true;			 
						     runPython();					     
						     newAiRunPython = false;
						     xSemaphoreGive(xRunPythonMutex);
						}	    
				 }
 
    } 
}

void USB_Download_Task(void *pvParameters)
{
    usbTimer = xTimerCreate(
        "USBTimer",                   
        pdMS_TO_TICKS(3),           
        pdFALSE,                      
        (void *)0,                   
        USB_IdleTimeoutCallback         
     );

    blueTimer = xTimerCreate(
        "BLUETimer",                   
        pdMS_TO_TICKS(3),           
        pdFALSE,                      
        (void *)0,                   
        BLUE_IdleTimeoutCallback         
     );						
    while(1) {
        EventBits_t uxEvents = xEventGroupWaitBits(
            xEventGroup,
            EVENT_USB_FRAM_BYTE|EVENT_BLUE_FRAM_BYTE|EVENT_PLAYER_KEY_VIOC,
            pdTRUE,       
            pdFALSE,      
            portMAX_DELAY
        );
      
			  if(xSemaphoreTake(xUsbMutex, portMAX_DELAY) == pdPASS) {
					usbDownloadActive = true;
					if(uxEvents & EVENT_USB_FRAM_BYTE) {
            downloadFile(USB,getUSB_RingBuffer_Handle());  
					}
					if(uxEvents & EVENT_BLUE_FRAM_BYTE)
					{ 
           downloadFile(BLUE,getBLUE_RingBuffer_Handle());
					}	
           if((uxEvents & EVENT_PLAYER_KEY_VIOC))
          {
							extern void LinkePlayerVIOC(void);
                            LinkePlayerVIOC();
          }					
					usbDownloadActive = false;
					xSemaphoreGive(xUsbMutex);
				}
			 	taskYIELD();
    }
}
 
void MatChineStateTask(void *param)
{

    EventBits_t uxEvents;

    xTaskCreate(USB_Download_Task, "USB_Download", 4*1024, NULL, 
               TASK_PRIO_USB_DOWNLOAD, NULL);
	
    xTaskCreate(EnteryTask, "MastEntery", 8*1024, NULL, 
               TASK_PRIO_USB_DOWNLOAD, NULL);
	
	  extern void bspInit(void);
	  bspInit();		
	  blue_init();
	  redrawUIInit(NULL,true);
	  init_portdata_struct();  
  
	  taskENTER_CRITICAL();
    xKeyScanTimer = xTimerCreate(
        "KeyScan",                 
        pdMS_TO_TICKS(10),          
        pdTRUE,                    
        (void *)0,                  
        KeyScanTimerCallback      
     );
    xPortScanTimer = xTimerCreate(
        "PortScan",
        pdMS_TO_TICKS(50),
        pdTRUE,
        (void *)1,
        PortScanTimerCallback
    );   
    xMonitorSendTimer = xTimerCreate(
        "MonitorSend",
        pdMS_TO_TICKS(30),        
        pdTRUE,
        (void *)3,
        MonitorSendTimerCallback
    );
    xBatSendTimer = xTimerCreate(
        "BatScant",
        pdMS_TO_TICKS(100),        
        pdTRUE,
        (void *)4,
        BatSendTimerCallback
    );
    xBlueSendTimer = xTimerCreate(
        "BlueOnOff",
        pdMS_TO_TICKS(200),        
        pdTRUE,
        (void *)5,
        BlueSendTimerCallback
    );	
    xIWDGTimer = xTimerCreate(
        "IWDGMonitor",
        pdMS_TO_TICKS(50),        
        pdTRUE,
        (void *)6,
        IWDG_FreeTimeoutCallback
    );	
    remoteTimer = xTimerCreate(
        "Remote",
        pdMS_TO_TICKS(2000),        
        pdFALSE,
        (void *)7,
        REMOTE_TimeoutCallback
    );		
	 
    xMemTimer = xTimerCreate(
        "mem",                  
        pdMS_TO_TICKS(20),          
        pdTRUE,                     
        (void *)8,                 
        MemUpdata_TimeoutCallback      
     );			
				
    /*ai model init*/				
		//colorAi_init();
				
	  extern void iwdg_init(uint32_t prer, uint16_t rlr);
		iwdg_init(IWDG_PRESCALER_64,500);		
 
    if(xPortScanTimer != NULL) xTimerStart(xPortScanTimer, 0);
    if(xMonitorSendTimer != NULL) xTimerStart(xMonitorSendTimer, 0);		
    if(xBatSendTimer != NULL)	xTimerStart(xBatSendTimer, 0);		
		if(xBlueSendTimer!=NULL) xTimerStart(xBlueSendTimer, 0);					
		if(xKeyScanTimer != NULL) xTimerStart(xKeyScanTimer, 0);		
		if(xIWDGTimer != NULL) xTimerStart(xIWDGTimer, 0);	
		if(xMemTimer !=NULL) xTimerStart(xMemTimer,0);		
		taskEXIT_CRITICAL();		
		extern void MX_USB_DEVICE_Init(void);
		MX_USB_DEVICE_Init();				
    while(1) {
        uxEvents = xEventGroupWaitBits(
            xEventGroup,
            EVENT_REFRESH_MATRIX|
			      EVENT_FIND_PORT_DEV|
						EVENT_KEY_ENTERY|
			      EVENT_SEND_MONITOR|
			      EVENT_BLUE_STATE_REFRESH|
						EVENT_BAT_REFRESH|
			      EVENT_CAMER_MODE,
            pdTRUE,       
            pdFALSE,       
            portMAX_DELAY
        );

        if(!usbDownloadActive) {
            if(uxEvents & EVENT_REFRESH_MATRIX) {
							if(xSemaphoreTake(xRefreshMutex, portMAX_DELAY) == pdPASS) {
								   refreshMatrix = true;
								   redrawMatrixUI(NULL);
								   refreshMatrix = false;
								   xSemaphoreGive(xRefreshMutex);
							}						   
            } 
            if(uxEvents & EVENT_SEND_MONITOR) {
                newAiMonitor();						 
            }
            if(uxEvents & EVENT_CAMER_MODE)
            {
               if(getRunState() !=true)
                   set_hw_camer_mode();    
            }
            if(uxEvents & EVENT_FIND_PORT_DEV) {
                 FindProtDev();
            }
						if((uxEvents & EVENT_KEY_ENTERY)){ 
						    ui_entery();
						}
						if(uxEvents & EVENT_MEM_REFRESH) { 		
							 lsm6ds3tr_c_motion_fx_determin();	
						}
						if((uxEvents & EVENT_BAT_REFRESH)){ 
							_show_write_led(
                (unsigned char*)getBatLevel(bat_level - 1),
                CalculatedBrightness(UI_DEFAULT_COLOR, UI_DEFAULT_BRIGTNESS)
							);						    
						}
 
        }
        taskYIELD();
    }
}
 

static void KeyScanTimerCallback(TimerHandle_t xTimer) {
    if(!usbDownloadActive) {
        updata_key_value();
    }	
		extern void usb_check_plug_state(void);
		usb_check_plug_state();
		 
}
 
static void PortScanTimerCallback(TimerHandle_t xTimer) {
    if(!usbDownloadActive) {
        SET_EVENT_GROUP(EVENT_FIND_PORT_DEV);
    }
}
static void MonitorSendTimerCallback(TimerHandle_t xTimer) {
    if(!usbDownloadActive && !USB_IDLE_TIME_FLAG && !BLUE_IDLE_TIME_FLAG && is_monitor) {		  
       SET_EVENT_GROUP(EVENT_SEND_MONITOR);   		
    }	 
 
}
static void BatSendTimerCallback(TimerHandle_t xTimer)
{ 
    extern Filter_t voic_bat_filters[2];
    if(!usbDownloadActive) 
    {
        uint16_t vref[2];
        ScanBat_VoicVREF(vref);
        
        const float ADC_TO_VOLTAGE = 3.3f / 65535.0f;
        const float VOLTAGE_SCALE = 151.0f / 51.0f; // Լ����2.96
        const float MIN_VOLTAGE = 7.0f;/**/
        const float VOLTAGE_RANGE = 1.2f;
        
        float raw_voltage = vref[0] * ADC_TO_VOLTAGE;
        float filtered_voltage = UpdataFilterPro(&voic_bat_filters[0], raw_voltage);

        float scaled_voltage = filtered_voltage * VOLTAGE_SCALE;
        scaled_voltage = (scaled_voltage > MIN_VOLTAGE) ? scaled_voltage : MIN_VOLTAGE;
        
        bat_percent = ((scaled_voltage - MIN_VOLTAGE) / VOLTAGE_RANGE) * 100.0f;
        bat_percent = (bat_percent < 0.0f) ? 0.0f : (bat_percent > 100.0f) ? 100.0f : bat_percent;
        
          bat_level = (uint8_t)(bat_percent / 25.0f) + 1;
        bat_level = (bat_level < 1) ? 1 : (bat_level > 4) ? 4 : bat_level;
        
        raw_voltage = vref[1] * ADC_TO_VOLTAGE;
        voic = UpdataFilterPro(&voic_bat_filters[1], raw_voltage);
  
				if(xSemaphoreTake(xRefreshMutex, portMAX_DELAY) == pdPASS) {
						if(getCurrentUiList() == BAT_LOGO && !refreshMatrix)
						{ 
							SET_EVENT_GROUP(EVENT_BAT_REFRESH); 			 
						}		   
						xSemaphoreGive(xRefreshMutex);				
				}		
    }
}

static void BlueSendTimerCallback(TimerHandle_t xTimer)
{ 
	  
	  if(!usbDownloadActive) {
    if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_10) == 1)
   {
			 BUS_LINE = true;
       HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_SET);			  		 
   }
   else
   {			 
			 if(blueHandle()->blueState == 1)
			 { 
			   HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_6);
			 }
       else
			 { 
			   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_RESET);		
			 }
			 BUS_LINE = false;		 				
   }
   if(BUS_LINE)
   { 
		  if(blueHandle()->blueMonitorBuffer!=NULL && !get_remote_linke()){ 
			   BLUE_printf("%s\r\n",blueHandle()->blueMonitorBuffer);
				 memset(blueHandle()->blueMonitorBuffer,0,2*1024);
			}
	    
	 }		 
	} 
}

static void USB_IdleTimeoutCallback(TimerHandle_t xTimer)
{ 
   process_received_data();
	 USB_IDLE_TIME_FLAG = false;
}
static void BLUE_IdleTimeoutCallback(TimerHandle_t xTimer)
{ 
   SET_EVENT_GROUP(EVENT_BLUE_FRAM_BYTE);	
   BLUE_IDLE_TIME_FLAG = false;
}
static void IWDG_FreeTimeoutCallback(TimerHandle_t xTimer)
{ 
  extern void iwdg_feed(void);
  iwdg_feed();
}
static void REMOTE_TimeoutCallback(TimerHandle_t xTimer)
{ 
   clearRemoteValue();
}
static void MemUpdata_TimeoutCallback(TimerHandle_t xTimer)
{ 
	 if(!usbDownloadActive) {
	 SET_EVENT_GROUP(EVENT_MEM_REFRESH);  
	 }
}

