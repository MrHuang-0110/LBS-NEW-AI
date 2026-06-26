#include "_music.h"
#include "music.h"
#include "PikaVM.h"
#include "devfile.h"
static char musicPath[32];
static MusicCommand_t cmd = {0};
volatile bool py_music_state = false;
void _music_setvolumeto(PikaObj *self, int val)
{ 
  setmusicvolumento((float)val);
}
pika_float _music_soundIntensity(PikaObj *self)
{ 
  return 0;
}
void _music_soundPianomusic(PikaObj *self, char* val, pika_float sduty)
{ 
	  
	 size_t fileObjSize = 0;
	
   fullmusicpath(musicPath);
	 strcat(musicPath,val);
	
	 if(getFileObjSize(musicPath,&fileObjSize) !=FR_OK)return;
	 
	 if(cmd.music_data!=NULL){vPortFree(cmd.music_data);cmd.music_data = NULL;}
	 
	 cmd.music_data = (uint8_t*)pvPortMalloc(fileObjSize);
	 if(cmd.music_data == NULL)return;
	
	 memset(cmd.music_data,0,fileObjSize);
	 if(ExternFlashReadFile(musicPath,cmd.music_data)!=FR_OK)return;
	 
	 cmd.type = NEW_MUSIC_DATA;
	 cmd.sduty = sduty;
	 cmd.data_length = fileObjSize;
	 py_music_state = true;
	 xMusicSendQueueCmd(&cmd);
	
	 while(py_music_state && VMSignal_getCtrl() != 1){ 
            pika_GIL_EXIT();
            vTaskDelay(10);
            pika_GIL_ENTER();	   
	 }
	 vPortFree(cmd.music_data);cmd.music_data = NULL;
}
void _music_start(PikaObj *self, char* val)
{ 
	 fullmusicpath(musicPath);
	 strcat(musicPath,val);
	
	 size_t fileObjSize = 0;
   
	 if(cmd.music_data!=NULL){vPortFree(cmd.music_data);cmd.music_data = NULL;}
 
	 
	 if(getFileObjSize(musicPath,&fileObjSize) !=FR_OK){return;} 
 
	 cmd.music_data = (uint8_t*)pvPortMalloc(fileObjSize);
	 if(cmd.music_data == NULL){return;} 
	
	 memset(cmd.music_data,0,fileObjSize);
	 if(ExternFlashReadFile(musicPath,cmd.music_data)!=FR_OK){return;} 
	 
	 cmd.type = NEW_MUSIC_DATA;
	 cmd.sduty = 1;
	 cmd.data_length = fileObjSize;
	 xMusicSendQueueCmd(&cmd);
}
void _music_stop(PikaObj *self)
{ 
   if(cmd.music_data!=NULL){vPortFree(cmd.music_data);cmd.music_data = NULL;}
		
	 cmd.type = STOP_MUSIC;
	 cmd.sduty = 1;
	 cmd.data_length = 0;
	 xMusicSendQueueCmd(&cmd);
}
void _music_untildone(PikaObj *self, char* val)
{ 
	 size_t fileObjSize = 0;
	
   fullmusicpath(musicPath);
	 strcat(musicPath,val);
	
	 if(getFileObjSize(musicPath,&fileObjSize) !=FR_OK)return;
	 
	 if(cmd.music_data!=NULL){vPortFree(cmd.music_data);cmd.music_data = NULL;}
	 
	 cmd.music_data = (uint8_t*)pvPortMalloc(fileObjSize);
	 if(cmd.music_data == NULL)return;
	
	 memset(cmd.music_data,0,fileObjSize);
	 if(ExternFlashReadFile(musicPath,cmd.music_data)!=FR_OK)return;
	 
	 cmd.type = NEW_MUSIC_DATA;
	 cmd.sduty = 1;
	 cmd.data_length = fileObjSize;
	 py_music_state = true;
	 xMusicSendQueueCmd(&cmd);
	
	 while(py_music_state && VMSignal_getCtrl() != 1){ 
            pika_GIL_EXIT();
            vTaskDelay(10);
            pika_GIL_ENTER();	   
	 }
	 vPortFree(cmd.music_data);cmd.music_data = NULL;
}
