#include "rawMatrix.h"
//#include "malloc.h"
#include "string.h"
#include "music.h"
#include "devfile.h"
#include "gpio.h"
#include "matrix.h"
#include "dblist.h"
#include "key.h"
#include "dac.h"
#include "uart.h"
#include "matchineState.h"

extern TIM_HandleTypeDef htim6;
static DoublyLinkedList *uiList;

static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) music_buffer[512]; 
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) key_Button_buffer[8*1024];
static uint8_t __attribute__((section(".DMA_SRAM1_32KB"))) usb_Connect_buffer[14*1024];
 
static void bubbleSort(uint8_t *b,int Size)
{
    int i = 0, j = 0,m = 0, temp = 0;
    for( i = 0; i < Size; i++)
    {
        m = i; 
        for( j = i+1; j < Size; j++) 
            if(b[j] < b[m])        
                m = j;   
        if(m != i)
        {
            temp = b[i];
            b[i] = b[m];
            b[m] = temp;
        }
    }
}
 
void PowerStopLogo(void)
{
    
   unsigned char DruyBmp[3][9] = {
                        {0xFE,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0xFE},
                        {0x00,0x00,0x00,0x38,0x28,0x38,0x00,0x00,0x00},
                        {0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00}};
	 uint8_t lamp[9];
												
 
   uint8_t druyOffset = 0;
	 
   memset(lamp,0,sizeof(lamp));		
												
   /*清屏*/												
   _show_write_led(lamp,0);
   druyOffset = 0;
   uint32_t musicSize;
	 WAVheader header;
	 unsigned int br;
	 _IO_FILE *file = getFatfsHandle();
	 char res = FR_DISK_ERR;	
	 res = f_open(&file->ioFile,"1:music/PowerDownNEW.wav",FA_READ);  
	 if(res != FR_OK)
	 {
			f_close(&file->ioFile);
			return;
	 } 	 
	 BSP_KEY *key = getKeyHandle();
	// myfree(SRAM12,key->keyMusic);
	// myfree(SRAM12,key->LinkeMusic);
//	 uint8_t *data = (uint8_t *)mymalloc(SRAM12,file->ioFile.obj.objsize);
	// if(data == NULL)while(1);
//	 memset(data,0,file->ioFile.obj.objsize);
	 res = f_read(&file->ioFile,&header,sizeof(header),&br);
	 musicSize = header.ChunkSize-0x2C;
	 HAL_TIM_Base_Start_IT(&htim6); 
	 HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 
	 do
	 {
 	    if((musicSize - 6144)>512)
			{
			   res = f_read(&file->ioFile,music_buffer,512,&br);
          if(res == FR_OK &&
             br == 512)
          {
	            musicSize-=512;
			        dac_out_put(music_buffer,512);
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
 
			int color = CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS);
			
			if(druyOffset<3)
					_show_write_led(DruyBmp[druyOffset++],color);			
			
			
			extern void iwdg_feed(void);
			iwdg_feed();			
	  }while(musicSize);
    HAL_TIM_Base_Stop_IT(&htim6);
    HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);							
	//  myfree(SRAM12,data);
	  f_close(&file->ioFile);
		
	 memset(lamp,0,sizeof(lamp));		
		
	 _DEV_CFG *cfg_handle = getCfgFileHandle();
		
		if(cfg_handle!=NULL)
		{ 
			  
		 ExternFlashWrite("1:system.cfg", 
                      (const char*)cfg_handle, 
                       sizeof(_DEV_CFG));	
		}
		 
   /*清屏*/												
   _show_write_led(lamp,0);
}

void StartLogo(void)
{ 
	 
   uint8_t lamp[9];
	 uint32_t musicSize;
	 WAVheader header;
	 unsigned int br;
	 _IO_FILE *file = getFatfsHandle();
	 char res = FR_DISK_ERR;	
	  
	 res = f_open(&file->ioFile,"1:music/Start.wav",FA_READ);  
	 if(res != FR_OK)
	 {
			f_close(&file->ioFile);
			return;
	 } 	 
	// uint8_t *data = (uint8_t *)mymalloc(SRAM12,file->ioFile.obj.objsize);
 
	// if(data == NULL)while(1);
	// memset(data,0,file->ioFile.obj.objsize);
	 res = f_read(&file->ioFile,&header,sizeof(header),&br);
	 musicSize = header.ChunkSize-0x2C;
	 HAL_TIM_Base_Start_IT(&htim6); 
	 HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_SET);	 
 
	 #define MUSICE_RATE 1
	 do
	 {
 	    if((musicSize - 6144)>512 * MUSICE_RATE)
			{
			   res = f_read(&file->ioFile,music_buffer,512 * MUSICE_RATE,&br);
          if(res == FR_OK &&
             br == 512 * MUSICE_RATE)
          {
	            musicSize-=512 * MUSICE_RATE;
			        dac_out_put(music_buffer,512 * MUSICE_RATE);
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
            for(uint8_t i = 0;i<9;i++)
								lamp[i] = rand() % 0xFF;
 
			int color = CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS);
			_show_write_led(lamp,color);			
	  }while(musicSize);
	 
		
    HAL_TIM_Base_Stop_IT(&htim6);
    HAL_GPIO_WritePin(SoundPower_IO_GPIO_Port, SoundPower_IO_Pin, GPIO_PIN_RESET);							
	//  myfree(SRAM12,data);
	  f_close(&file->ioFile);
		
		#if 1
		lamp[0] = 0x66;
		lamp[1] = 0x66;
		lamp[2] = 0x00;
		lamp[3] = 0xCC;
		lamp[4] = 0xCC;
		lamp[5] = 0xCC;
		lamp[6] = 0xCC;
		lamp[7] = 0xFC;
		lamp[8] = 0x38;
		uint8_t brigtness = 1;
		for (uint8_t i = 0; i < 12; i++) {
		  int color = CalculatedBrightness(UI_DEFAULT_COLOR, brigtness++);
		  _show_write_led(lamp, color);
			vTaskDelay(50);
    }				
    #endif
		/*加载按键音效资源*/
		BSP_KEY *key = getKeyHandle();
    res = f_open(&file->ioFile,"1:music/Button.wav",FA_READ);  
	  if(res != FR_OK)
	  {
			f_close(&file->ioFile);
			return;
	  } 	 		
	  res = f_read(&file->ioFile,&header,sizeof(header),&br);
	  musicSize = header.ChunkSize-0x2C;	
    key->keyMusicSize = musicSize;		
		key->keyMusic = key_Button_buffer;
	  //key->keyMusic = (uint8_t *)mymalloc(SRAM12,musicSize);
	 // if(key->keyMusic == NULL)while(1);
	  memset(key->keyMusic,0,8*1024);		
	  f_read(&file->ioFile,key->keyMusic,musicSize,&br);
	  f_close(&file->ioFile);
 
      res = f_open(&file->ioFile,"1:music/continue.wav",FA_READ);  
	  if(res != FR_OK)
	  {
			f_close(&file->ioFile);
			return;
	  } 	
    memset(&header,0,sizeof(WAVheader)); 		
	  res = f_read(&file->ioFile,&header,sizeof(header),&br);
	  musicSize = header.ChunkSize-0x2C;	
    key->LinkeMusicSize = musicSize;		
	  //key->LinkeMusic = (uint8_t *)mymalloc(SRAM12,musicSize);
		key->LinkeMusic = usb_Connect_buffer;
	 // if(key->LinkeMusic == NULL)while(1);
	  memset(key->LinkeMusic,0,14*1024);		
	  f_read(&file->ioFile,key->LinkeMusic,musicSize,&br);
	  f_close(&file->ioFile);
 
}
uint8_t getCurrentUiList(void)
{
  _DEV_CFG *cfgFile = getCfgFileHandle(); 
   return get_at_index(uiList,cfgFile->defaultKeyUI); 
}
uint8_t get_ui_num(void)
{ 
   _DEV_CFG *cfgFile = getCfgFileHandle(); 
	 return (cfgFile->UInum - 3);
}
uint8_t get_ui_file(uint8_t key)
{ 
   return get_at_index(uiList,key); 
}
uint8_t getDefaultKey(void)
{ 
	 _DEV_CFG *cfgFile = getCfgFileHandle(); 
   return cfgFile->defaultKeyUI;
}
void sendUIlistNumber(uint8_t id)
{ 
	_DEV_CFG *cfgFile = getCfgFileHandle(); 
  MultiUart_SendFrame((UartDevice_t )id,&cfgFile->UInum,1,0x98,0xE7,10,250);
}

void redrawMatrixUI(char *filename)
{ 
   _DEV_CFG *cfgFile = getCfgFileHandle(); 
	 
	 BSP_KEY *key = getKeyHandle();
	 cfgFile->UInum = uiList->size;
	 if(cfgFile->defaultKeyUI > cfgFile->UInum)
		      cfgFile->defaultKeyUI = 0;
   
	 if(filename!=NULL)
	 { 
		  char number[10];
		  memset(number,0,sizeof(number));
		  sscanf(filename, "%*[^/]/%[^.].o", number);
		  int index = find_index_by_data(uiList,atoi(number));
		  if(index >= 0)
				cfgFile->defaultKeyUI = (uint8_t)index; 
	 }
	 
	 int uinum = get_at_index(uiList,cfgFile->defaultKeyUI); 
   if(uinum == (-1))return;
	 
	 _show_roll_ui(uinum);	 
	 keyPlayerVIOC();
     //LinkePlayerVIOC();
}
void refreshdefaultKeyUi(char *filename)
{ 
   _DEV_CFG *cfgFile = getCfgFileHandle(); 
	 BSP_KEY *key = getKeyHandle();
	 cfgFile->UInum = uiList->size;
	 if(cfgFile->defaultKeyUI > cfgFile->UInum)
		      cfgFile->defaultKeyUI = 0;
   
	 if(filename!=NULL)
	 { 
		  char number[10];
		  memset(number,0,sizeof(number));
		  sscanf(filename, "%*[^/]/%[^.].o", number);
		  int index = find_index_by_data(uiList,atoi(number));
		  if(index >= 0)
				cfgFile->defaultKeyUI = (uint8_t)index; 
			/*run py*/
			extern void ui_entery(void);
			ui_entery();
	 }  
	 
}
void redrawUIInit(char*filename,bool state)
{ 
	 uint8_t onum[20],osum = 0;
	
	 if(uiList!=NULL)
	     destroy_list(uiList);
	 
   uiList = create_list();
	 if(uiList == NULL)while(1);
	 
	 memset(onum,0,sizeof(onum));
   uint8_t ret;
   _IO_FILE *file = getFatfsHandle(); 
   ret = f_opendir(&file->ioDir,"1:app");
	 while(1)
	 {
	      ret = f_readdir(&file->ioDir, &file->ioinfo);
	      if (ret != FR_OK || file->ioinfo.fname[0] == 0) break;
        if(file->ioinfo.fattrib & AM_DIR) {;}
		    else
		    {
		       int len = strlen(file->ioinfo.fname);
			     if (len <= 5 &&
			   	     file->ioinfo.fname[len - 2] == '.' &&
               file->ioinfo.fname[len - 1] == 'o'
               )
            {
							char numStr[3];
							strncpy(numStr,file->ioinfo.fname,2);
							numStr[2] = '\0';
							int num = atoi(numStr);
							if(num >= 0 && num <= 19 )													
								 onum[osum++] = num;					
            }
	      }
	  }
		f_closedir(&file->ioDir); 
		bubbleSort(onum,osum);	
    for(uint8_t i = 0;i<osum;i++)
		    insert_tail(uiList,onum[i]);
		
		insert_tail(uiList,REMOTRE_LOGO);
		insert_tail(uiList,BLUE_LOGO);
		insert_tail(uiList,BAT_LOGO);
 
		if(state)
			redrawMatrixUI(filename);
		else
			refreshdefaultKeyUi(filename);
}
