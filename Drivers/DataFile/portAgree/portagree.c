#include "portagree.h"
//#include "malloc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "uart.h"
#include "ultrasion.h"
#include "touch.h"
#include "motor.h"
#include "mem.h"
#include "gray.h"
#include "color.h"
#include "adc.h"
#include "nfc_car.h"
//#include "cjSON.h"
#include "json-maker.h"
#include "devfile.h"
#include "blue.h"
#include "ringbufer.h"
#include "matchineState.h"
#include "ringbufer.h"
#include "camer.h" 
#include "grayv2.h"

static __PORT portDev[8];
static FrameParser frame_parser;
static char json_buffer[10*1024];

extern volatile bool SENSORD_STATE;

__PORT *getPortHandle(uint8_t index)
{ 
   return &portDev[index];
}

SensorBase *getDevBase(uint8_t index)
{ 
   return portDev[index].sensors;
}
void init_portdata_struct(void)
{ 
   for(uint8_t i = 0;i<8;i++)
	 { 
	   portDev[i].portTimeOutTick = 0;
		 portDev[i].sensors = NULL;
		 portDev[i]._LastLinkeObjDev = 0;
		 portDev[i]._LinkeObjDev = 0;
 
	 }
	  frame_parser_init(&frame_parser);

}
 

 
 
static void port_linke(int portIndex,int id,uint8_t *data)
{
     if(strcmp((const char*)data,
			         (const char*)"Play Aplication") == 0)
      {
				  SENSORD_STATE = false;		
         		
			    if(identify_and_bind(&portDev[portIndex],id,portIndex) == true)
					{
					     portDev[portIndex]._LinkeObjDev = id;		
					}
				
      }
      else
			{
					portDev[portIndex]._LinkeObjDev = 0xEF;
				  SENSORD_STATE = true;
			}
}


static void port_data_parsing(int portIndex,int id,int index,uint8_t *data)
{ 
	 /*download file.c waite function */
   switch(index)
	 {
		 case DEV_PORT_LINKE:port_linke(portIndex,id,data);break;
 
		 case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x10:case 0x0C:case 0x0D:
		 case DEV_PORT_INFO_DATA:
			  portDev[portIndex].sensors->findIndex = index;
			  set_sensor_parameter(portDev[portIndex].sensors,data);			
     break;

		 /*sensord updata*/
		 /*version ack*/
		 case 0x08:
			 if(strcmp((const char*)data,"Write Version Error") == 0)
									SENSORD_STATE = false;
			 else
									SENSORD_STATE = true;
	   break;
		 /*false waite ack*/
		 case 0xAA:			 
		 if(strcmp((const char*)data,"Flash Write OK") == 0)
				SENSORD_STATE = true;
		 break;
		 /*flash data end*/
		 case 0xBB:
			  SENSORD_STATE = true;			 
		 break;
		 
		 default:break;
	 }
     if(portDev[portIndex].sensors->type == DEV_ID_CAMER)
     {
        portDev[portIndex].portTimeOutTick = 50;
     }
		 else if(portDev[portIndex].sensors->type == DEV_ID_GRAY_V2)
		 { 
		     portDev[portIndex].portTimeOutTick = 50;
		 }
		 else
     {
        portDev[portIndex].portTimeOutTick = 10;
     }
	  
}

 
uint8_t calculate_checksum(const uint8_t *data, size_t length) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum&0xFF;
}

uint8_t dataAgreeAnalys(_AGREEMENT *_agreement_,uint8_t *data,uint16_t length)
{ 
  if(data[0] != 0x5A && data[length - 1] != 0xA5)
   	  return AGREE_MEN_ERROR;

   if(length <8)
		  return AGREE_MEN_ERROR;

	 static uint8_t _mycrc_;
   _mycrc_ = calculate_checksum((const uint8_t *)data,length - 2);

   if(_mycrc_!=data[length - 2])
	 {
	    return AGREE_MEN_ERROR;
	 }
   else
   {
		 memset(_agreement_->data,0,256);
     _agreement_->Head = data[0];
	   _agreement_->sID = data[1];
	   _agreement_->oID = data[2];
	   _agreement_->length = data[3];
     _agreement_->index = data[4];

     memcpy(_agreement_->data,data+5,data[3]);

	   _agreement_->crc = data[length - 2];
	   _agreement_->tard = data[length - 1];
	  return AGREE_MEN_OK;
   }
}

void FreeDevReFrechSource(uint8_t port)
{ 
 
			switch(portDev[port]._LinkeObjDev)
		{ 
	   case DEV_ID_BIG_MOTOR:
	 	   free_dev_motor(port);
		 break;
		 case DEV_ID_SMALL_Motor:	
				free_dev_motor(port);
		 break;	 
		 case DEV_ID_GRAY:
			  free_gray(port);
		 break;	 
		 case DEV_ID_TOUCH:
				freeTouch(port);
		 break;	 
		 case DEV_ID_COLOR:
			 free_color(port);
		 break;	 
		 case DEV_ID_ULTRASION:
			 free_ultrasion(port);
		 break;	 
		 case DEV_ID_CAMER:
			 free_camer(port);
		 break; 
		 case DEV_ID_NFC:
			 free_dev_nfc(port);
		 break;
		 case DEV_ID_GRAY_V2:
			 free_gray_v2(port);
		 break;
		}
	  portDev[port]._LinkeObjDev = 0;
		if(portDev[port].sensors != NULL)
		{ 
			 portDev[port].sensors = NULL;			  
		}  
}
void ResetScanPort(void)
{ 
    for(uint8_t i = 0;i<8;i++)
	 { 		    
	   if(portDev[i].portTimeOutTick > 0)
		 {
		   portDev[i].portTimeOutTick--;
			 if(portDev[i].portTimeOutTick == 0)
			 {
					 FreeDevReFrechSource(i);			 
			 }
		 }
     if(portDev[i].ultrasionTick > 0)
		 {
		    portDev[i].ultrasionTick--;
		 }
	 }
}
// 初始化帧解析器
void frame_parser_init(FrameParser *parser) {
	  
    parser->state = STATE_IDLE;
    parser->index = 0;
    parser->expected_length = 0;
    parser->calc_checksum = 0;
    parser->frame_valid = false;
    memset(parser->buffer, 0, MAX_FRAME_SIZE);
}

// 处理接收到的字节
bool frame_parser_process_byte(FrameParser *parser, uint8_t byte) {
    switch (parser->state) {
        case STATE_IDLE:
            if (byte == FRAME_HEADER) {
                parser->state = STATE_HEADER;
                parser->index = 0;
                parser->calc_checksum = 0;
                parser->buffer[parser->index++] = byte;
                parser->calc_checksum += byte;  // 初始化校验和
            }
            break;
            
        case STATE_HEADER:
            if (byte == SRC_ID) {
                parser->state = STATE_SRC_ID;
                parser->buffer[parser->index++] = byte;
                parser->calc_checksum += byte;
            } else {
                frame_parser_init(parser);
            }
            break;
            
        case STATE_SRC_ID:
            if (byte == DEST_ID) {
                parser->state = STATE_DEST_ID;
                parser->buffer[parser->index++] = byte;
                parser->calc_checksum += byte;
            } else {
                frame_parser_init(parser);
            }
            break;
            
        case STATE_DEST_ID:
            parser->state = STATE_LENGTH;
            parser->buffer[parser->index++] = byte;
            parser->calc_checksum += byte;
            parser->expected_length = byte;  // 长度字段
            break;
            
        case STATE_LENGTH:
            parser->state = STATE_TYPE;
            parser->buffer[parser->index++] = byte;
            parser->calc_checksum += byte;
            parser->frame_type = byte;  // 类型字段
            
            // 检查是否有数据字段
            if (parser->expected_length == 0) {
                parser->state = STATE_CHECKSUM;
            }
            break;
            
        case STATE_TYPE:
            // 这个状态应该只在expected_length>0时才会进入
            parser->state = STATE_DATA;
            parser->buffer[parser->index++] = byte;
            parser->calc_checksum += byte;
            parser->data_bytes_received = 1;  // 跟踪已接收的数据字节数
            
            // 检查是否已经接收完所有数据
            if (parser->data_bytes_received >= parser->expected_length) {
                parser->state = STATE_CHECKSUM;
            }
            break;
            
        case STATE_DATA:
            parser->buffer[parser->index++] = byte;
            parser->calc_checksum += byte;
            parser->data_bytes_received++;
            
            // 检查是否接收完所有数据
            if (parser->data_bytes_received >= parser->expected_length) {
                parser->state = STATE_CHECKSUM;
            }
            break;
            
        case STATE_CHECKSUM:
            parser->buffer[parser->index++] = byte;
            
            // 验证校验和
            if (byte == (parser->calc_checksum & 0xFF)) {
                parser->state = STATE_FOOTER;
            } else {
                frame_parser_init(parser);
            }
            break;
            
        case STATE_FOOTER:
            parser->buffer[parser->index++] = byte;
            if (byte == FRAME_FOOTER) {
                // 完整帧接收成功
                parser->frame_valid = true;
                return true;
            } else {
                frame_parser_init(parser);
            }
            break;
            
        default:
            frame_parser_init(parser);
            break;
    }
    
    // 检查缓冲区溢出
    if (parser->index >= MAX_FRAME_SIZE) {
        frame_parser_init(parser);
    }
    
    return false;
}

void process_received_data(void) {
    uint8_t byte = 0x0;     
	  extern RingBuffer *getPROCES_RingBuffer_Handle(void);
    RingBuffer *handle = getPROCES_RingBuffer_Handle();
	  if(handle!=NULL)
		{ 
			while (ring_buffer_get(handle, &byte)) {
        if (frame_parser_process_byte(&frame_parser, byte)) {
						 
            handle_complete_frame(
                frame_parser.buffer, 
                frame_parser.index,
                frame_parser.frame_type,
                frame_parser.expected_length
            );
            
					  frame_parser_init(&frame_parser);
        }
			}		   
		}
}

void handle_complete_frame(uint8_t *frame, uint16_t length, uint8_t type, uint16_t data_length) {
    uint8_t *frame_data = &frame[5];
    uint8_t byte,crc = 0;
	  extern RingBuffer *getUSB_RingBuffer_Handle(void);
	  RingBuffer *handle = getUSB_RingBuffer_Handle();
	
	  byte = 0x5A;
	  crc+=0x5A;
	  ring_buffer_write(handle,&byte,1);
	  byte = 0x97;
	  crc+=0x97;
	  ring_buffer_write(handle,&byte,1);
	  byte = 0x98;
	  crc+=0x98;
	  ring_buffer_write(handle,&byte,1);	
	  byte = data_length;
	  crc+=data_length;
	  ring_buffer_write(handle,&byte,1);		
	  byte = type;
	  crc+=type;
	  ring_buffer_write(handle,&byte,1);		
	  
		for(uint8_t i = 0;i<data_length;i++)
		{ 
		   crc+=frame_data[i];
		}
    ring_buffer_write(handle,frame_data,data_length);		
	  crc&=0xFF;
	  ring_buffer_write(handle,&crc,1);
		byte = 0xA5;
		ring_buffer_write(handle,&byte,1);	
		
		SET_EVENT_GROUP(EVENT_USB_FRAM_BYTE);   
}

void scan_agreement_data_port_dev(uint8_t index,uint8_t *pData,uint16_t length)
{ 
   _AGREEMENT rxAGREEMENT;
	 if(index == BLUE)
	 { 		 
			extern TimerHandle_t blueTimer;
			extern volatile bool BLUE_IDLE_TIME_FLAG;		 
		  extern RingBuffer *getBLUE_RingBuffer_Handle(void);
		  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		  if(getBLUE_RingBuffer_Handle()!=NULL)
			{ 
				  ring_buffer_write(getBLUE_RingBuffer_Handle(),pData,length);
				  //SET_EVENT_GROUP_ISR(EVENT_BLUE_FRAM_BYTE);	
				  #if 1
					if(BLUE_IDLE_TIME_FLAG == false)
					{ 
						xTimerStartFromISR(blueTimer, &xHigherPriorityTaskWoken);
						BLUE_IDLE_TIME_FLAG = true;
					}		 
					else
						xTimerResetFromISR(blueTimer, &xHigherPriorityTaskWoken);	
         #endif					
			}
		  return;
	 }
	 
	 if(dataAgreeAnalys(&rxAGREEMENT,pData,length)!=AGREE_MEN_OK)
	 {
				if(portDev[index]._LinkeObjDev == DEV_ID_ULTRASION)
				{		
				 if(portDev[index].sensors != NULL)
				 {
					  set_sensor_parameter(portDev[index].sensors,pData);					 
				 }
				}		
	 }
	 else
	 {
      port_data_parsing(index,rxAGREEMENT.sID,rxAGREEMENT.index,rxAGREEMENT.data);		  
	 }
}
 
void FindProtDev(void)
{
	  extern Filter_t adc_filters[8];
		extern DeviceDetector_t device_detectors[8];
    extern float vdda_voltage; 
			
    float filtered_voltage = 0.0f;
    uint16_t port_vref[8];
	  uint8_t detected_type;
    UartDevice_t devId;
    
    ScanPortRevf(port_vref);
		
    for(uint8_t i = 0; i < 8; i++)
    {
        devId = (UartDevice_t)i;    

        float raw_voltage = (float)(port_vref[i] * 3.3f / 65535.0f);      
        filtered_voltage = UpdateFilter(&adc_filters[i], raw_voltage);
        if(portDev[i]._LinkeObjDev == 0)
        {
						detected_type = DetectDeviceAdaptive(&device_detectors[i], filtered_voltage);
            portDev[i]._LinkeObjDev = detected_type;        
            if(portDev[i]._LinkeObjDev != portDev[i]._LastLinkeObjDev)
            {
                portDev[i]._LastLinkeObjDev = portDev[i]._LinkeObjDev;
                
                if(portDev[i]._LinkeObjDev == DEV_ID_ULTRASION)
                {
											 changerUsartBaudrate(i, 9600);								
											 return;			 
                }
                else
                {	
											 changerUsartBaudrate(i, 115200);											
											 return;				
                }
            }
            MultiUart_SendFrame(devId, (uint8_t*)"Please Link",strlen("Please Link"),0x98,0x09, 10, 250);						
        }
        else
        {
 						if( portDev[i]._LinkeObjDev == 0xEF)
						{MultiUart_SendFrame(devId, (uint8_t*)"Please Link", strlen("Please Link"),0x98,0x09, 10, 250);return;}
            if(portDev[i]._LinkeObjDev == DEV_ID_ULTRASION)
            {
						  if(filtered_voltage > NONE_MIN && filtered_voltage < NONE_MAX)
							{ 
								if(portDev[i].sensors!=NULL)
								{ 
									portDev[i]._LinkeObjDev = 0;	
									portDev[i].sensors = NULL;	
								}
							}						 							
							if(portDev[i].sensors == NULL)
							{ 
								   identify_and_bind(&portDev[i], DEV_ID_ULTRASION,i);
							}
              else if(portDev[i].ultrasionTick == 0)
              {
                    uint8_t UltrasionCmd = 0xA0;
                    MultiUart_SendMoreByte(devId, &UltrasionCmd, 1, 10, 250);
                    portDev[i].ultrasionTick = 4;
              }
            }
            else if(portDev[i]._LinkeObjDev == DEV_ID_TOUCH)
            {
								if(filtered_voltage > NONE_MIN && filtered_voltage < NONE_MAX)
							 { 
								if(portDev[i].sensors!=NULL)
								{ 
									 portDev[i]._LinkeObjDev = 0;	
									 portDev[i].sensors = NULL;	
								}
							 }						
                if(portDev[i].sensors == NULL)
                {
                    identify_and_bind(&portDev[i], DEV_ID_TOUCH,i);
                }
                else
                {
                    uint8_t touchState = 0;
                    if(filtered_voltage > TOUCH_DOWN_MIN_VALUE && filtered_voltage < TOUCH_DOWN_MAX_VALUE)
                    {
                        touchState = 1;
                    }
                    set_sensor_parameter(portDev[i].sensors, &touchState);												
                }
            }				
        }
    }
}
#if 0
/* 外部串口打印函数，请根据实际平台修改 */
extern void USB_printf(const char *fmt, ...);

/* 将任务状态枚举转换为字符 */
static char taskStatusChar(eTaskState eState)
{
    switch(eState) {
        case eRunning:   return 'R';
        case eReady:     return 'r';
        case eBlocked:   return 'B';
        case eSuspended: return 'S';
        case eDeleted:   return 'D';
        default:         return '?';
    }
}

/* 获取运行时间总计数（用于计算百分比） */
static uint32_t getTotalRunTime(void)
{
    TaskStatus_t *pxTaskStatusArray = NULL;
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    uint32_t ulTotal = 0;

    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);
        for (UBaseType_t i = 0; i < uxArraySize; i++) {
            ulTotal += pxTaskStatusArray[i].ulRunTimeCounter;
        }
        vPortFree(pxTaskStatusArray);
    }
    return ulTotal;
}

void vPrintSystemStatus(void)
{
    USB_printf("\n========== System Status ==========\n");

    /* 获取任务数量 */
    UBaseType_t uxTaskCount = uxTaskGetNumberOfTasks();
    USB_printf("Total tasks: %u\n", uxTaskCount);

    /* 分配内存存储任务状态 */
    TaskStatus_t *pxTaskStatusArray = pvPortMalloc(uxTaskCount * sizeof(TaskStatus_t));
    if (pxTaskStatusArray == NULL) {
        USB_printf("Failed to allocate memory\n");
        return;
    }

    /* 获取所有任务的状态 */
    UBaseType_t uxActualCount = uxTaskGetSystemState(pxTaskStatusArray, uxTaskCount, NULL);
    if (uxActualCount != uxTaskCount) {
        USB_printf("Warning: got %u tasks but expected %u\n", uxActualCount, uxTaskCount);
    }

    /* 可选：计算总运行时间，用于 CPU 百分比 */
    uint32_t ulTotalRunTime = 0;
#if (configGENERATE_RUN_TIME_STATS == 1)
    for (UBaseType_t i = 0; i < uxActualCount; i++) {
        ulTotalRunTime += pxTaskStatusArray[i].ulRunTimeCounter;
    }
#endif

    /* 打印表头 */
    USB_printf("\n%-16s | St | Prio | Stack Free(B) | CPU %%\n", "Task Name");
    USB_printf("----------------+----+------+---------------+------\n");

    for (UBaseType_t i = 0; i < uxActualCount; i++) {
        TaskStatus_t *p = &pxTaskStatusArray[i];
        char cStatus = taskStatusChar(p->eCurrentState);

        /* 栈剩余转换为字节（假设栈深度单位为 4 字节） */
        uint32_t stackFreeBytes = p->usStackHighWaterMark * sizeof(StackType_t);

        /* 计算 CPU 百分比 */
        uint8_t cpuPercent = 0;
#if (configGENERATE_RUN_TIME_STATS == 1)
        if (ulTotalRunTime > 0) {
            cpuPercent = (p->ulRunTimeCounter * 100) / ulTotalRunTime;
        }
#endif

        USB_printf("%-16s | %c  | %4lu | %11lu | %3u%%\n",
                    p->pcTaskName,
                    cStatus,
                    (unsigned long)p->uxCurrentPriority,
                    stackFreeBytes,
                    cpuPercent);
    }

    vPortFree(pxTaskStatusArray);

    /* 可选：打印 vTaskList 和 vTaskGetRunTimeStats 作为补充（简洁版） */
    
    USB_printf("\n---- vTaskList() ----\n");
    vTaskList(json_buffer);
    USB_printf("%s", json_buffer);

#if (configGENERATE_RUN_TIME_STATS == 1)
    USB_printf("\n---- vTaskGetRunTimeStats() ----\n");
    vTaskGetRunTimeStats(json_buffer);
    USB_printf("%s", json_buffer);
#endif

    USB_printf("===================================\n\n");
}
#endif
void newAiMonitor(void) { 
	  #define DEBUG_INFO 0
	  
	  #if 1
    size_t remLen = 10*1024;
    char *p = json_buffer;
    char temp_str[64];


	  memset(p,0,10*1024);

    p = json_objOpen(p, NULL, &remLen);
    p = json_arrOpen(p, "deviceList", &remLen);
    
    for(uint8_t i = 0; i < 8; i++) {
        p = json_objOpen(p, NULL, &remLen);
        p = json_int(p, "port", i, &remLen);
        
        if(portDev[i].sensors != NULL) {			
            switch(portDev[i]._LinkeObjDev) {
							 				
                case DEV_ID_BIG_MOTOR:
                case DEV_ID_SMALL_Motor:
								{
                    DEV_MOTOR *devmotor = read_motor((SensorBase *)portDev[i].sensors);
                    const char* motor_type = (portDev[i]._LinkeObjDev == DEV_ID_BIG_MOTOR) ? 
                                           "big_motor" : "small_motor";								   
									
                    p = json_objOpen(p, motor_type, &remLen);
                    
                    snprintf(temp_str, sizeof(temp_str), "%.2f", motor_hw_get_circly(i));
                    p = json_str(p, "circly", temp_str, &remLen);
                     
                    snprintf(temp_str, sizeof(temp_str), "%d", motor_hw_get_speed(i));
                    p = json_str(p, "speed", temp_str, &remLen);
                    
								 
                   // snprintf(temp_str, sizeof(temp_str), "%d", motor_hw_get_position(i));
                   // p = json_str(p, "pos", temp_str, &remLen);
                    
                    snprintf(temp_str, sizeof(temp_str), "%d", motor_hw_get_angle(i));
                    p = json_str(p, "angle", temp_str, &remLen);
                    
                   // snprintf(temp_str, sizeof(temp_str), "%d", motor_hw_get_encords(i));
                   // p = json_str(p, "encord", temp_str, &remLen);

                    snprintf(temp_str, sizeof(temp_str), "%.2f", motor_hw_get_dt(i));
                    p = json_str(p, "dt", temp_str, &remLen);
										
                    if(strcmp(motor_type,"kt_motor") == 0)
                    {
                        snprintf(temp_str, sizeof(temp_str), "%d", 0);
                         p = json_str(p, "absAngle", temp_str, &remLen);                        
                    }
                    snprintf(temp_str, sizeof(temp_str), "%d", motor_hw_get_version(i));
                    p = json_str(p, "version", temp_str, &remLen);
                    
                    uint16_t software_version;
                    getdevSoftware(portDev[i]._LinkeObjDev, &software_version);
                    snprintf(temp_str, sizeof(temp_str), "%d", software_version);
                    p = json_str(p, "SoftwareVersion", temp_str, &remLen);
                    p = json_objClose(p, &remLen);
                    break;
                }
                case DEV_ID_ULTRASION: {
                    DEV_ULTRASION *devultrasion = read_ultrasion((SensorBase *)portDev[i].sensors);
                    p = json_objOpen(p, "ultrasion", &remLen);                   
                    snprintf(temp_str, sizeof(temp_str), "%.2f", devultrasion->cm);
                    p = json_str(p, "cm", temp_str, &remLen);
                    p = json_objClose(p, &remLen);
                    break;
                }
                case DEV_ID_TOUCH: {
                    DEV_TOUCH *devtouch = read_touch((SensorBase *)portDev[i].sensors);
                    p = json_objOpen(p, "touch", &remLen);
                    p = json_int(p, "state", devtouch->touchState, &remLen);						
                    p = json_objClose(p, &remLen);
                    break;
                }
                case DEV_ID_GRAY: {
                    DEV_GRAY *devgray = read_gray((SensorBase *)portDev[i].sensors);
                    p = json_objOpen(p, "gray", &remLen);
                    p = json_int(p, "1", devgray->L1, &remLen);
                    p = json_int(p, "2", devgray->L2, &remLen);
                    p = json_int(p, "3", devgray->R1, &remLen);
                    p = json_int(p, "4", devgray->R2, &remLen);
                    p = json_int(p, "b1", devgray->L1State, &remLen);
                    p = json_int(p, "b2", devgray->L2State, &remLen);
                    p = json_int(p, "b3", devgray->R1State, &remLen);
                    p = json_int(p, "b4", devgray->R2State, &remLen);
                    p = json_int(p, "version", devgray->version, &remLen);
                    
                    uint16_t software_version;
                    getdevSoftware(portDev[i]._LinkeObjDev, &software_version);
                    p = json_int(p, "Softwareversion", software_version, &remLen);               
                    p = json_objClose(p, &remLen);
                    break;
                }
                case DEV_ID_COLOR: {
                    DEV_COLOR *devcolor = read_color((SensorBase *)portDev[i].sensors);
                    p = json_objOpen(p, "color", &remLen);
                    p = json_int(p, "r", devcolor->RGBL[0], &remLen);
                    p = json_int(p, "g", devcolor->RGBL[1], &remLen);
                    p = json_int(p, "b", devcolor->RGBL[2], &remLen);
									
                  //  if(devcolor->version >=138)
									//	{ 
									//		snprintf(temp_str, sizeof(temp_str), "%.2f", devcolor->sensor_data.filtered);
									//	  p = json_str(p, "l", temp_str, &remLen);
									//		p = json_int(p, "s", devcolor->sensor_data.current_line, &remLen);
								//		}								 
									//	else 
											p = json_int(p, "lux", devcolor->RGBL[3], &remLen);
										//  p = json_int(p, "rwalux", devcolor->RGBL[4], &remLen);                               
                     
                    p = json_int(p, "version", devcolor->version, &remLen);
                    
                    uint16_t software_version;
                    getdevSoftware(portDev[i]._LinkeObjDev, &software_version);
                    p = json_int(p, "SoftwareVersion", software_version, &remLen);      
                    p = json_objClose(p, &remLen);
                    break;
                }
								case DEV_ID_NFC:{ 
								    DEV_NFC *dev_nfc = read_nfc((SensorBase *)portDev[i].sensors);
										p = json_objOpen(p, "nfc", &remLen);
                    p = json_int(p, "id", dev_nfc->car_id, &remLen);
                    p = json_int(p, "version", dev_nfc->version , &remLen);
                    p = json_objClose(p, &remLen);
										break;
								}
								case DEV_ID_GRAY_V2:{ 
									 DEV_GRAYV2 *dev_gray_v2 = read_gray_v2((SensorBase *)portDev[i].sensors);
										p = json_objOpen(p, "gray_v2", &remLen);
                    p = json_int(p, "1", dev_gray_v2->L_Value[0], &remLen);
                    p = json_int(p, "2", dev_gray_v2->L_Value[1], &remLen);
                    p = json_int(p, "3", dev_gray_v2->L_Value[2], &remLen);
                    p = json_int(p, "4", dev_gray_v2->L_Value[3], &remLen);
                    p = json_int(p, "5", dev_gray_v2->L_Value[4], &remLen);
                    p = json_int(p, "6", dev_gray_v2->L_Value[5], &remLen);
									  p = json_int(p, "7", dev_gray_v2->L_Value[6], &remLen);
                    p = json_int(p, "b1", dev_gray_v2->L_State[0], &remLen);
                    p = json_int(p, "b2", dev_gray_v2->L_State[1], &remLen);
                    p = json_int(p, "b3", dev_gray_v2->L_State[2], &remLen);
                    p = json_int(p, "b4", dev_gray_v2->L_State[3], &remLen);
                    p = json_int(p, "b5", dev_gray_v2->L_State[4], &remLen);
                    p = json_int(p, "b6", dev_gray_v2->L_State[5], &remLen);
                    p = json_int(p, "b7", dev_gray_v2->L_State[6], &remLen);
                    p = json_int(p, "t1", dev_gray_v2->Threshold[0], &remLen);
                    p = json_int(p, "t2", dev_gray_v2->Threshold[1], &remLen);
                    p = json_int(p, "t3", dev_gray_v2->Threshold[2], &remLen);
                    p = json_int(p, "t4", dev_gray_v2->Threshold[3], &remLen);
                    p = json_int(p, "t5", dev_gray_v2->Threshold[4], &remLen);
                    p = json_int(p, "t6", dev_gray_v2->Threshold[5], &remLen);
                    p = json_int(p, "t7", dev_gray_v2->Threshold[6], &remLen);
                    p = json_int(p, "version", dev_gray_v2->version, &remLen);                 
                    uint16_t software_version;
                    getdevSoftware(portDev[i]._LinkeObjDev, &software_version);
                    p = json_int(p, "Softwareversion", software_version, &remLen);               
                    p = json_objClose(p, &remLen);									 
								   break;
								}
								case DEV_ID_CAMER:
								{
								DEV_CAMER *dev_camer = read_camer((SensorBase *)portDev[i].sensors);
								p = json_objOpen(p,"camer",&remLen);
								 p = json_int(p, "mode",dev_camer->mode, &remLen);
								 switch((uint8_t)dev_camer->mode)
								 {
									case CAMER_MENU_TYPE:
									case CAMER_MODE_TYPE:
									case CAMER_FACE_TYPE:
									case CAMER_LABE_TYPE:
									case CAMER_OBJECT_TYPE:
									case CAMER_COLOR_TYPE:
									case CAMER_WAY_TYPE:
									case CAMER_GESTURE_TYPE:
									case CAMER_BODY_TYPE:
									case CAMER_OBJECT_BODY_TYPE:
									case CAMER_PHOTO_TYPE:	
									p = json_int(p, "id1",dev_camer->data[0], &remLen); 
									p = json_int(p, "x",dev_camer->data[1]<<8|dev_camer->data[2], &remLen); 
									p = json_int(p, "y",dev_camer->data[3]<<8|dev_camer->data[4], &remLen); 
									p = json_int(p, "w",dev_camer->data[5]<<8|dev_camer->data[6], &remLen); 
									p = json_int(p, "h",dev_camer->data[7]<<8|dev_camer->data[8], &remLen); 
									p = json_int(p, "pp",dev_camer->data[9], &remLen); 

									p = json_int(p, "id2",dev_camer->data[10], &remLen); 
									p = json_int(p, "x",dev_camer->data[11]<<8|dev_camer->data[12], &remLen); 
									p = json_int(p, "y",dev_camer->data[13]<<8|dev_camer->data[14], &remLen); 
									p = json_int(p, "w",dev_camer->data[15]<<8|dev_camer->data[16], &remLen); 
									p = json_int(p, "h",dev_camer->data[17]<<8|dev_camer->data[18], &remLen); 
									p = json_int(p, "pp",dev_camer->data[19], &remLen); 

									p = json_int(p, "id3",dev_camer->data[20], &remLen); 
									p = json_int(p, "x",dev_camer->data[21]<<8|dev_camer->data[22], &remLen); 
									p = json_int(p, "y",dev_camer->data[23]<<8|dev_camer->data[24], &remLen); 
									p = json_int(p, "w",dev_camer->data[25]<<8|dev_camer->data[26], &remLen); 
									p = json_int(p, "h",dev_camer->data[27]<<8|dev_camer->data[28], &remLen); 
									p = json_int(p, "pp",dev_camer->data[29], &remLen); 
							 
									p = json_int(p, "id4",dev_camer->data[30], &remLen); 
									p = json_int(p, "x",dev_camer->data[31]<<8|dev_camer->data[32], &remLen); 
									p = json_int(p, "y",dev_camer->data[33]<<8|dev_camer->data[34], &remLen); 
									p = json_int(p, "w",dev_camer->data[35]<<8|dev_camer->data[36], &remLen); 
									p = json_int(p, "h",dev_camer->data[37]<<8|dev_camer->data[38], &remLen); 
									p = json_int(p, "pp",dev_camer->data[39], &remLen);
									 break;
								 }
								 p = json_objClose(p, &remLen);
								 break;                                   
								}
            }
 						
        } else {
            if(portDev[i]._LinkeObjDev == 0xEF) {
                p = json_objOpen(p, "dev null", &remLen);
                p = json_objClose(p, &remLen);
            }
        }
        
        p = json_objClose(p, &remLen); // 关闭端口对象
    }
    
    p = json_arrClose(p, &remLen); // 关闭设备列表数组
    
 
		
    // 添加Flash信息
    uint32_t total, free_flash;
    getFlashTotalFree(&total, &free_flash);
    p = json_objOpen(p, "flash", &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f mb", total/1024.0f);
    p = json_str(p, "total", temp_str, &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f mb", free_flash/1024.0f);
    p = json_str(p, "free", temp_str, &remLen);
    
    p = json_objClose(p, &remLen);
    
    // 添加NewAi版本信息
    uint16_t version;
    getNewAiSoftware(&version);
    p = json_int(p, "version", version, &remLen);
    
    // 添加内存信息
    DEV_MEM *dev_mem = getMemHandle();
    p = json_objOpen(p, "mem", &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f", dev_mem->yaw);
    p = json_str(p, "yaw", temp_str, &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f", dev_mem->pitch);
    p = json_str(p, "pitch", temp_str, &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f", dev_mem->roll);
    p = json_str(p, "roll", temp_str, &remLen);
		
    p = json_objClose(p, &remLen);
    
    // 添加堆信息
    size_t free_heap = xPortGetFreeHeapSize();
    snprintf(temp_str, sizeof(temp_str), "%d", free_heap);
    p = json_str(p, "heap", temp_str, &remLen);
    
    // 添加电池和声音信息
    extern float bat_percent;
    extern float voic;
    
    snprintf(temp_str, sizeof(temp_str), "%.2f", bat_percent);
    p = json_str(p, "bat", temp_str, &remLen);
    
    snprintf(temp_str, sizeof(temp_str), "%.2f", voic);
    p = json_str(p, "voic", temp_str, &remLen);
    
    // 添加MAC地址
    p = json_str(p, "MAC", blueHandle()->BLUE_MAC, &remLen);
 
		snprintf(temp_str,sizeof(temp_str),"%s","NewAiState");
		if(getRunState())
			p = json_str(p, temp_str,"run", &remLen);
		else
			p = json_str(p, temp_str,"stop", &remLen);
		
		#if 0
    snprintf(temp_str, sizeof(temp_str), "%d", my_mem_perused(SRAMIN));
		p = json_str(p, "ram1:",temp_str,&remLen);
		
    snprintf(temp_str, sizeof(temp_str), "%d", my_mem_perused(SRAM12));
		p = json_str(p, "ram2:",temp_str,&remLen);

    snprintf(temp_str, sizeof(temp_str), "%d", my_mem_perused(SRAM4));
		p = json_str(p, "ram3:",temp_str,&remLen);

    snprintf(temp_str, sizeof(temp_str), "%d", my_mem_perused(SRAMDTCM));
		p = json_str(p, "ram4:",temp_str,&remLen);

    snprintf(temp_str, sizeof(temp_str), "%d", my_mem_perused(SRAMITCM));
		p = json_str(p, "ram5:",temp_str,&remLen);
		#endif
		
    // 关闭根对象
    p = json_objClose(p, &remLen);
    
    // 结束JSON
    p = json_end(p, &remLen);
    
    // 检查是否有错误
    if (p == NULL || remLen == 0) {
        // 缓冲区不足，处理错误
        return;
    }
    USB_printf("%s\r\n", json_buffer);	
    if(blueHandle()->blueMonitorBuffer!=NULL)
			strcpy(blueHandle()->blueMonitorBuffer,json_buffer);
		#else
		vPrintSystemStatus();
		#endif
}
