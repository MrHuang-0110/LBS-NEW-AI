#include "music.h"
//#include "malloc.h"
#include "string.h"
#include "devfile.h"
#include "gpio.h"
#include "matrix.h"
#include "dac.h"
#include "rawMatrix.h"
#include "key.h"
extern TIM_HandleTypeDef htim6;
static _IO_MUSIC music;
static QueueHandle_t xMusicQueue = NULL;
static TaskHandle_t  xMusicTask = NULL;  
static bool is_playing = false;

void music_player_task(void)
{ 
	 static MusicState_t current_music = {0};
	 
	 is_playing = false;	 
    while(getRunState())
    {
        MusicCommand_t command;
        if(xQueueReceive(xMusicQueue, &command, 0) == pdPASS)
        {
            if(command.type == NEW_MUSIC_DATA) // 接收到新音乐命令
            {      
                // 保存新音乐数据
                current_music.data = command.music_data;
                current_music.position = 0;
							  current_music.sduty = command.sduty;
							  current_music.length = command.data_length;
							  is_playing = true;     
								HAL_TIM_Base_Start_IT(&htim6); 
								HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 							
            }
            else if(command.type == STOP_MUSIC) // 停止命令
            {
                is_playing = false;
                musicStop();
            }
        }
				else
				{ 
					
						if(is_playing)
					{
						if((current_music.length-6144) > 512)
						{ 
						  playerMusicFram(current_music.data + current_music.position, 512);
							current_music.length-=512;
							current_music.position+=512;
						}
						else
						{ 
               current_music.position = 0;
							 is_playing = false;    
							 extern volatile bool py_music_state;
							 py_music_state = false;
							 musicStop();						
						}
            vTaskDelay((TickType_t)(current_music.sduty * 10));
					}
					else
         {
            vTaskDelay(100);
         }					
			}
    }
}
void player_button_music(void)
{ 
	 BSP_KEY *key = getKeyHandle();
	 HAL_TIM_Base_Start_IT(&htim6); 
	 HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 	 
   dac_out_put(key->keyMusic,key->keyMusicSize - 6144);	  
   HAL_TIM_Base_Stop_IT(&htim6);
   HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);				
}
void player_music(char *wav_path)
{ 
	 static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) data_buffer[512];
	
	 uint32_t musicSize;
	 WAVheader header;
	 unsigned int br;
	 _IO_FILE *file = getFatfsHandle();
	 char res = FR_DISK_ERR;	
	 res = f_open(&file->ioFile,wav_path,FA_READ);  
	 if(res != FR_OK)
	 {
			f_close(&file->ioFile);
			return;
	 } 	 
	 //uint8_t *data = (uint8_t *)mymalloc(SRAM12,512);
	 //if(data == NULL)while(1);
	 //memset(data,0,512);
	 memset(data_buffer,0,512);
	 res = f_read(&file->ioFile,&header,sizeof(header),&br);
	 musicSize = header.ChunkSize-0x2C;
	 HAL_TIM_Base_Start_IT(&htim6); 
	 HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 
	 do
	 {
 	    if((musicSize - 6144)>512)
			{
			   res = f_read(&file->ioFile,data_buffer,512,&br);
          if(res == FR_OK &&
             br == 512)
          {
	            musicSize-=512;
			        dac_out_put(data_buffer,512);
          }
          else
          {
             musicSize = 0;
             break;
          }

			   }
			   else
			   {
				      musicSize = 0;
              break;
			   }		
	  }while(musicSize);
	 
    HAL_TIM_Base_Stop_IT(&htim6);
    HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);							
	 // myfree(SRAM12,data);
	  f_close(&file->ioFile);    
}
void playerMusicFram(uint8_t *pData,uint16_t length)
{ 
    static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) data_buffer[512];
 	   for(uint16_t Loop = 0;Loop<length;Loop++)
	  {
	    pData[Loop] *= music.volumento / 100.0f;
	  } 
		//uint8_t *data = (uint8_t *)mymalloc(SRAM12,length);
	//	if(data == NULL){return;} 
	//	memset(data,0,length);	
		memset(data_buffer,0,length);	
		memcpy(data_buffer,pData,length);
		dac_out_put(data_buffer,length);
	//	myfree(SRAM12,data);
}
void setmusicvolumento(float volumento)
{ 
  music.volumento = volumento>100?100:volumento;
}
void musicInfoInit(void)
{ 
  music.volumento = 100.0f;
}
void musicStop(void)
{ 
    HAL_TIM_Base_Stop_IT(&htim6);
    HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);	  
}
void fullmusicpath(char *musicpath)
{ 
   memset(musicpath,0,32);
	sprintf(musicpath,"%s","1:music/");
} 
void xMusicSendQueueCmd(MusicCommand_t *cmd)
{ 
	  if(xMusicQueue!=NULL && cmd!=NULL)
		{ 
			 xQueueSend(xMusicQueue,cmd, pdMS_TO_TICKS(10));
		   vTaskDelay(pdMS_TO_TICKS(1));
		}			   
}
void create_music_play_task(void)
{ 
	if(xMusicQueue == NULL)
	{ 
	   xMusicQueue = xQueueCreate(16, sizeof(MusicCommand_t));
	}
	if(xMusicTask == NULL)
	{ 
	   xTaskCreate((TaskFunction_t	)music_player_task, "musicplayer", 1024, NULL, 5, &xMusicTask);
	}
}	
void delet_music_player(void)
{ 
	if(xMusicQueue!=NULL)
	{ 
	    vQueueDelete(xMusicQueue);
		  xMusicQueue = NULL;
	}
  if(xMusicTask!=NULL)
	{ 
	   vTaskDelete(xMusicTask);  // 删除任务
		 xMusicTask = NULL;
	} 
}
