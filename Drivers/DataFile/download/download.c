#include "download.h"
//#include "malloc.h"
#include "uart.h"
#include "music.h"
#include "devfile.h"
#include "matrix.h"
#include "rawMatrix.h"
#include "blue.h"
#include "devfile.h"
#include "PikaCompiler.h"
#include "remote.h"
#include "motor.h"
#include "color.h"
#include "gray.h"
#include "entery.h"
 
#include "ctype.h"
static RingBuffer USB_RingBufer;
static RingBuffer BLUE_RingBufer;
static RingBuffer PROCES_RingBufer;
static DOWNLOAD_FILE iofile;
volatile bool SENSORD_STATE = false;

static uint8_t __attribute__((section(".DTCM_Data"))) fileBufer[32*1024];
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) usb_ring_bufer[512];
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) blue_ring_bufer[512];
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) proces_ring_bufer[512];
 
RingBuffer *getUSB_RingBuffer_Handle(void)
{  
    return &USB_RingBufer;
}
RingBuffer *getBLUE_RingBuffer_Handle(void)
{  
    return &BLUE_RingBufer;
}
RingBuffer *getPROCES_RingBuffer_Handle(void)
{ 
		return &PROCES_RingBufer;
}
// 文件保存函数（示例实现）
static bool saveFileToStorage(const char* path, uint8_t* data, size_t size)
{
    if(ExternFlashWrite((char*)path,(const char*)data,size) == FR_OK)
        return true;
		else
			  return false;
}
static bool writle_sensord_dataToAck(uint8_t portIndex,uint8_t *data,uint16_t length,uint8_t cmd)
{
    int waiteTimeOut = 0;
 
	  SENSORD_STATE = false;
    while(!SENSORD_STATE){
	       MultiUart_SendFrame((UartDevice_t )portIndex,data,length,0x98,cmd,10,250);
         vTaskDelay(50);
         waiteTimeOut++;
         if(waiteTimeOut > 100)
         {
              break;
         }
    }
    if(waiteTimeOut > 100)
        return false;
    else
        return true;
}

static bool writle_sensord_none_loop(uint8_t portIndex,uint8_t *data,uint16_t length,uint8_t cmd)
{
    int waiteTimeOut = 0;
	  SENSORD_STATE = false;
	  MultiUart_SendFrame((UartDevice_t )portIndex,data,length,0x98,cmd,10,250);
    while(!SENSORD_STATE){     
         waiteTimeOut++;
         if(waiteTimeOut > 10)
         {
              break;
         }
				  vTaskDelay(200);
    }
    if(waiteTimeOut > 10)
		{
//			USB_printf("[Error][%x][%x][%s]Send Error length[%d]\r\n",portIndex,cmd,data,length);
		  return false;}     
    else
        return true;
}

 static void sensord_updata(char *port)
{
	  float sumSize = 0;
	  char softwareVersion[32];
	  bool is_move;
	  uint8_t DEV_SENSORD_STATE,ERROR_COUNT,_move_led;
	  uint8_t res;
		uint32_t fileSize,offSetAddr;	
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为 pdPASS */   
	 /*1.clear matrix led*/
	  _clear_matrix();
	  player_music("1:music/devUpdata.wav");
	 /*2.Reset Dev*/
	  uint8_t i = 0;
		for(i = 0;i<8;i++)
		{ 
			 if(port[i] == 0xFF)
				    continue;
			 
			 DEV_SENSORD_STATE = 0;ERROR_COUNT = 0;res = 0;offSetAddr = 0;is_move = true;_move_led = 0;memset(softwareVersion,0,sizeof(softwareVersion));
			 __PORT *dev_port = getPortHandle(i); 
			 while(is_move)
			 { 
				 switch(DEV_SENSORD_STATE)
			  {
				 case 0:
				   MultiUart_SendFrame((UartDevice_t )i,"1",1,0x98,0xEE,10,250); 
					 vTaskDelay(250);
				   writle_sensord_none_loop(i,(uint8_t*)"Please Link",strlen("Please Link"),0x09);		
					 vTaskDelay(50);	
				   if(dev_port->_LinkeObjDev == 0xEF || SENSORD_STATE)
					 { DEV_SENSORD_STATE = 1;refresmatrtixlamp(1,i); ERROR_COUNT = 0;} 
					 if(ERROR_COUNT < 5)ERROR_COUNT++;
					 else{ is_move = false;showError(i);break;} 
				 break;
					 
				 case 1:
					 dev_port->_LinkeObjDev = 0;
				   if(!writle_sensord_none_loop(i,"0",1,0x08))
					 { 
					   if(ERROR_COUNT < 5)ERROR_COUNT++;
						 else 
						 { is_move = false;showError(i);break;} 
					 }
					 else
					 { 
						  DEV_SENSORD_STATE = 2;
					    ERROR_COUNT = 0;	
              refresmatrtixlamp(1,i);						 
					 }
				 break;
					 
				 case 2:
            memset(iofile.fileData,0,32*1024);
					  res = _read_sensord_bin(port[i],iofile.fileData,&fileSize);
						if(res!=FR_OK)
						{writle_sensord_none_loop(i,"0",1,0x08); showError(i);is_move = false;break;}				
						 sumSize = fileSize/1024.0f;
            DEV_SENSORD_STATE = 3;						
				 break;
				 
				 case 3:		
 
					   if(fileSize > 128)
					   {
							  
							 if(!writle_sensord_none_loop(i,&iofile.fileData[offSetAddr],128,0xAA))
								{writle_sensord_none_loop(i,"0",1,0x08); showError(i);is_move = false;break;}	
							 offSetAddr+=128;
							 fileSize-=128;
					   }
					   else
					   {
						   if(!writle_sensord_none_loop(i,&iofile.fileData[offSetAddr],fileSize,0xAA))
							 {writle_sensord_none_loop(i,"0",1,0x08); showError(i);is_move = false;break;}						 
							 offSetAddr+=fileSize;
							 fileSize-=fileSize;
							 DEV_SENSORD_STATE = 4;
							 	
					   }
							 RollingLed(1,i,++_move_led,UI_DEFAULT_COLOR);
							 if(_move_led == 2){
								  _move_led = 0;
								  RollingLed(1,i,0,UI_DEFAULT_COLOR);
								  RollingLed(0,i,1,0);
								  RollingLed(0,i,2,0);
							 }
							 USB_printf("[debug]->[%d]updata %.2f kb/%.2f kb\r\n",i,(float)(offSetAddr/1024.0f),sumSize);
				 break;
				 
				 case 4:
					 _read_sensord_versionfile(port[i],softwareVersion);
					if(!writle_sensord_none_loop(i,(uint8_t*)softwareVersion,strlen(softwareVersion),0xBB))
					{writle_sensord_none_loop(i,"0",1,0x08); showError(i);is_move = false;break;}	
  
           MultiUart_SendFrame((UartDevice_t )i,"1",1,0x98,0xFE,10,250);					
					 showOK(i);	
					 DEV_SENSORD_STATE = 5;				
           is_move = false;						
				 break;
				 
				 case 5:
				 break;
			  }				
			 }
		}
		 //unsigned char lamp[] = {0x00,0x6C,0x6C,0x00,0x82,0x82,0x82,0x7C,0x00};
		 //_show_write_led(lamp,CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS));	
//		 USB_printf("->Player Music\r\n");
		 player_music("1:music/DownLoadOK.wav");
	//	 USB_printf("->Refresh Matrix\r\n");
		 SET_EVENT_GROUP(EVENT_REFRESH_MATRIX);
  //   USB_printf("->OK Matrix\r\n");		 
}

 static void touchFile(uint8_t port,uint8_t index, uint8_t *data, uint16_t length)
{
	  extern void BLUE_printf(const char *format, ...);
    // 确保路径缓冲区不会溢出
    #define MAX_PATH_LEN 32

    bool state = false;
    uint8_t flag = 0x01;
    uint8_t error_flag = 0x01; // 错误响应标志
    UartDevice_t portType = (UartDevice_t)port;
 
    char *end;
	  long num;
    switch(index)
    { 
        /* 创建文件 */
        case 0xDA:
					  #if 0
            if(iofile.fileData) {
                myfree(SRAMIN, iofile.fileData);
                iofile.fileData = NULL;
            }
						#endif
  
      num = strtol((const char*)data, &end, 10); // 10表示十进制 
			if (end != (const char*)data && (*end == '\0' || !isdigit(*end))) {
          if(num >=20 )
					{ 
						 MultiUart_SendFrame(portType, &flag, 1,0x98,0xFE, 10, 250);
						 goto _EXIT;
					    
					}
						    
				} else {
					MultiUart_SendFrame(portType, &flag, 1,0x98,0xFE, 10, 250);
					goto _EXIT;
			}				    
            memset(iofile.fileData,0,32*1024);		
						memset(iofile.path,0,MAX_PATH_LEN);
						memset(iofile.name,0,MAX_PATH_LEN);
						iofile.objSize = 0;
						 
            //memset(&iofile, 0, sizeof(DOWNLOAD_FILE));
            strncpy(iofile.path, "1:app/", MAX_PATH_LEN - 1);
            strncat(iofile.path, (const char*)data, MAX_PATH_LEN - strlen(iofile.path) - 1);
            iofile.path[MAX_PATH_LEN - 1] = '\0';  
            strncpy(iofile.name, (const char*)data, sizeof(iofile.name) - 1);
            iofile.name[sizeof(iofile.name) - 1] = '\0';           
            iofile.objSize = 0;   
						if(port == BLUE){vTaskDelay(500);/*蓝牙的话需要等一下*/}								
            MultiUart_SendFrame(portType, &flag, 1,0x98,0xFD, 10, 250);
						
            break;
        
        /* 数据块传输 */
        case 0xAA:
            if(iofile.path[0] == '\0') {
                MultiUart_SendFrame(portType, &error_flag, 1,0x98,0xFE, 10, 250);
                break;
            }
            
            // 分配或扩展内存
						#if 0
            if(iofile.fileData == NULL) {
                iofile.fileData = (uint8_t*)mymalloc(SRAMIN, length);
            } else {
                // 重新分配内存时检查有效性
                uint8_t* newPtr = (uint8_t*)myrealloc(SRAMIN, iofile.fileData, iofile.objSize + length);
                if(newPtr) {
                    iofile.fileData = newPtr;
                } else {
                    // 内存分配失败处理
                    MultiUart_SendFrame(portType, &error_flag, 1, 0xFE, 10, 250);
                    break;
                }
            }
             
            // 检查内存是否分配成功
            if(iofile.fileData == NULL) {
                MultiUart_SendFrame(portType, &error_flag, 1, 0xFE, 10, 250);
                break;
            }
						#endif
            memcpy(iofile.fileData + iofile.objSize, data, length); 
            iofile.objSize += length;
						 if(port == BLUE){vTaskDelay(250);/*蓝牙的话需要等一下*/}
            MultiUart_SendFrame(portType, &flag, 1,0x98,0xFD, 10, 250);
            break;
        
        /* 下载完成（不执行） */
        case 0xBB:
				case 0xBC:
            // 检查文件是否已创建
            if(iofile.path[0] == '\0') {
                MultiUart_SendFrame(portType, &error_flag, 1,0x98,0xFE, 10, 250);
                break;
            }						
            
            // 追加最后的数据块（如果有）
            if(length > 0) {
							  #if 0
							  uint8_t* newPtr;
							  if(iofile.fileData == NULL)
								{ 
								  iofile.fileData = (uint8_t*)mymalloc(SRAMIN, length);
								   memcpy(iofile.fileData + iofile.objSize, data, length);
									 iofile.objSize += length;
								}								    
								else
								{ 
								   newPtr = (uint8_t*)myrealloc(SRAMIN, iofile.fileData, iofile.objSize + length);
                  if(newPtr) {
									 
                    iofile.fileData = newPtr;
                    memcpy(iofile.fileData + iofile.objSize, data, length);
                    iofile.objSize += length;
                   } else {
                    // 内存分配失败，继续处理但数据不完整									   
									  MultiUart_SendFrame(portType, &error_flag, 1, 0xFE, 10, 250);
									  goto _EXIT;
                  }  
								}
								#endif
								memcpy(iofile.fileData + iofile.objSize, data, length);
								iofile.objSize+=length;
								
            }else{ 
							;
						}
 
						uint8_t result;
						result = check_file_extension(iofile.path);
						if(result ==1)
						{ 
							/*.py*/
							replace_extension(iofile.path);
							if(CompileFile(iofile.path,(char *)iofile.fileData)!=0)
							{ 
								state = false;
								goto _EXIT;
							} 
							
							state = true;
						}
						else if(result == 2)
						{ 
							 /*.o*/
						   state = saveFileToStorage(iofile.path,iofile.fileData,iofile.objSize);
						}
             
						_EXIT:
            // 清理资源
						closeFatfs();
						
						if(state)
						{
							
						  unsigned char lamp[] = {0x00,0x6C,0x6C,0x00,0x82,0x82,0x82,0x7C,0x00};
							if(index == 0xBB){ 
								_show_write_led(lamp,CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS));								
								player_music("1:music/DownLoadOK.wav");		
							}
							MultiUart_SendFrame(portType, &flag, 1,0x98,0xFD, 10, 250);	
						}					 
						else
						{ 
							  
							  unsigned char lamp[] = {0x00,0x00,0x6C,0x6C,0x00,0x38,0x44,0x44,0x00};
								_show_write_led(lamp,CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS));
								Delete_File(iofile.path);
                MultiUart_SendFrame(portType, &error_flag, 1,0x98,0xFE, 10, 250);	
                player_music("1:music/DownLoadError.wav");								
						}
							 
						
            //myfree(SRAMIN, iofile.fileData);
            //iofile.fileData = NULL;
						if(index == 0xBC)
							redrawUIInit(iofile.path,false); 
						else
							redrawUIInit(iofile.path,true); 
						
            iofile.path[0] = '\0'; // 标记文件已关闭					 
            break;
        
        default:
            // 处理未知命令
            MultiUart_SendFrame(portType, &error_flag, 1,0x98,0xFE, 10, 250);
            break;
    }
}

 

void downloadFile(uint8_t port,RingBuffer *q)
{ 
	 extern TimerHandle_t xIWDGTimer;
	 extern TimerHandle_t remoteTimer;
	 extern TimerHandle_t xPortScanTimer;
   extern volatile bool monitor_state;
   extern TimerHandle_t xMonitorSendTimer;	
	 
	 
	 uint32_t length;
	 uint8_t data[256];
	 char path[32];
	 DEV_COLOR *color = NULL;
	
	 memset(data,0,sizeof(data));
	
   length = ring_buffer_read(q,data,512 - ring_buffer_space(q));
	 
	 _AGREEMENT rxAGREEMENT;
	 
	 if(AGREE_MEN_OK == dataAgreeAnalys(&rxAGREEMENT,data,length))
	 {
		  switch(rxAGREEMENT.index)
			{
				/*remote*/
				case 0xC1:
					 
				  refreshRemoteValue(rxAGREEMENT.data,rxAGREEMENT.length);
				  set_remote_linke();
					if(remoteTimer!=NULL)
					{ 
						if (xTimerIsTimerActive(remoteTimer) == pdFALSE) {
							xTimerStart(remoteTimer, 0);
						} else {
							xTimerReset(remoteTimer, 0);
					}	
				 }
				break;
				 
				/*hardware updata*/
			  case 0x6F:				
				  if(xIWDGTimer!=NULL)
					{				
               if(FATFS_MAKEFILE()){
							    xTimerStop(xIWDGTimer,0);
								  while(1);
							 } 
          }			
				break;
				
				/*senords updata*/
				case 0x32:				
					if(!getRunState())
						sensord_updata((char*)rxAGREEMENT.data);
				break;
					
				/*get ui list num*/
				case 0xE7:
					sendUIlistNumber(port);
				break;
				
				/*clear py file*/
				case 0xE8:	 
				 if(!getRunState())
				 { 
				    memset(path,0,sizeof(path));
						strcat(path,(const char*)"1:app/");			
						strcat(path,(const char*)rxAGREEMENT.data);							
						Delete_File((const char*)path);
						redrawUIInit(NULL,true);		    
				 }
				break;
				 
				/*clear all py file*/
				case 0xE9:
					if(!getRunState())
					{ 
					 	 Delete_AllFile();
					   redrawUIInit(NULL,true);  
					}
				break;
					
				/*run current py file*/
				case 0xB6:
				case 0xB9:
					ui_entery();
				break;
				
				/*start send monitor*/
				case 0xBA:
				  monitor_state=1;
					xTimerStart(xMonitorSendTimer, 0);
				break;
				/*stop send monitor*/	
				case 0xBE:
					 monitor_state = 0;
					 xTimerStop(xMonitorSendTimer, 0);
				break;
 
				default:
					if(!getRunState())
						touchFile(port,rxAGREEMENT.index,rxAGREEMENT.data,rxAGREEMENT.length);
				break;
			}    
	 }	 
	 else
	 { 
		  if(is_valid_at_command((const char*)data,length)!=false)
			{ 			 
			   is_get_at_data(blueHandle()->atData,data,length);   
	 
				   if (strstr((const char*)data, "+MAC") != NULL)
					 { 
					   memset(blueHandle()->BLUE_MAC,0,sizeof(blueHandle()->BLUE_MAC));
						 sscanf((const char*)data, "%*[^:]:%31s", blueHandle()->BLUE_MAC);
					 }
	 
         blueHandle()->atState = 1; 				
			}	   
	 }
//	  USB_printf("->exit USB DownLoad\r\n");		 
}
void initdownloadResource(void)
{ 
  ring_buffer_init(&USB_RingBufer,usb_ring_bufer,512);
	ring_buffer_init(&BLUE_RingBufer,blue_ring_bufer,512);
	ring_buffer_init(&PROCES_RingBufer,proces_ring_bufer,512);
	
//	if(iofile.fileData !=NULL)
	//{
	//   myfree(SRAMDTCM,iofile.fileData);
 //    iofile.fileData = NULL;		
//	}
	
	//iofile.fileData = mymalloc(SRAMDTCM,32*1024);
	iofile.fileData = fileBufer;
	//if(iofile.fileData == NULL)
		//while(1);
	memset(iofile.fileData,0,32*1024);	
}
