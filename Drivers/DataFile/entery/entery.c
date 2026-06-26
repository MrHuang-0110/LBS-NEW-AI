#include "entery.h"
#include "rawMatrix.h"
#include "blue.h"
#include "pikaObj.h"
#include "pikascript.h"
#include "pikaVm.h"
#include "pikaParser.h"
#include "PikaCompiler.h"
#include "pikaScript.h"
#include "devfile.h"
//#include "malloc.h"
#include "matchineState.h"
#include "matrix.h"
#include "strlist.h"
#include "mem.h"
#include "os.h"
const unsigned char blue_logo[] = {0x00,0x18,0x54,0x34,0x18,0x14,0x34,0x58,0x00};
void power_close(void)
{ 
  //close_blueAndFeedIWDG();
  PowerStopLogo();
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
static void InitBSPdev(void)
{ 
   _ui_row_refresh();
	
	 extern void clear_matrix_cach(void);
	 clear_matrix_cach();
	
   extern void InitMotorBSP(void);
	 InitMotorBSP();
	
	 extern void create_music_play_task(void);
	 create_music_play_task();
	
	 python_matrix_init();
	
	 create_user_list();
	
	 resetyaw();
	 
	 extern uint32_t pyTick;
	 pyTick = 0;
}
static void closeBSPdev(void)
{ 
   //extern bool motor_controller_stop(void);
  // motor_controller_stop();
	 extern void motor_close_all(void);
	 motor_close_all();
	
	 extern void delet_music_player(void);
	 delet_music_player();
	 
	 delete_user_list();
   //extern void set_hw_camer_mode(void);
  // set_hw_camer_mode();
}
static void ClearPikaThoneTaskHandleByName(void) {
    UBaseType_t uxArraySize, x;
    TaskStatus_t *pxTaskStatusArray;
    uint32_t ulTotalRunTime;

    uxArraySize = uxTaskGetNumberOfTasks();
   // pxTaskStatusArray = mymalloc(SRAMIN,uxArraySize * sizeof(TaskStatus_t));
			pxTaskStatusArray	= pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if(pxTaskStatusArray != NULL) {
			  memset(pxTaskStatusArray,0,uxArraySize * sizeof(TaskStatus_t));
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,&ulTotalRunTime);
        for(x = 0; x < uxArraySize; x++) {
            if(strcmp("pika_thread", pxTaskStatusArray[x].pcTaskName) == 0) {
               vTaskDelete(pxTaskStatusArray[x].xHandle);
            }
        }
       // myfree(SRAMIN,pxTaskStatusArray);
				  vPortFree(pxTaskStatusArray);
    }
}

void runPython(void)
{ 
	  uint8_t uiListNum = getCurrentUiList();
	 size_t objsize;
 	 char path[32];
	 uint8_t *pData = NULL;
	 memset(path,0,sizeof(path));
	 sprintf(path,"1:app/%d.o",uiListNum); 
	  if(f_stat(path, NULL) != FR_OK){ 
			  return;
		} 
	 if(getFileObjSize(path,&objsize) != FR_OK){ 
	      goto EXIT;
	 } 
	// pData = (uint8_t*)mymalloc(SRAMIN,objsize);
	 pData = (uint8_t*)pvPortMalloc(objsize);
	 if(pData == NULL)
	 { 
	    goto EXIT;
	 }    
	 if(ExternFlashReadFile(path,pData)!=FR_OK)
	 { 
	   goto EXIT;
	 }
    
	 InitBSPdev();
   
	 PikaObj* PythonMain;
	 PythonMain = newRootObj("pikaMain", New_PikaMain);
	 pikaVM_runByteCodeInconstant(PythonMain, (uint8_t*)pData);
	 obj_deinit(PythonMain);
  
	 EXIT:
	 //myfree(SRAMIN,pData);	 	  
	 vPortFree(pData);
	 closeBSPdev();
	 ClearPikaThoneTaskHandleByName();
	 _show_roll_ui(uiListNum); 
}

void ui_entery(void)
{ 
	 extern volatile bool newAiRunPython;
	 uint8_t uiListNum = getCurrentUiList();
	  
	 /*ÔËÐÐpy½Å±¾*/
   if(uiListNum < 20 || uiListNum == REMOTRE_LOGO)
	 { 
		   if(getRunState() !=true)
	        SET_EVENT_GROUP(EVENT_RUN_PYTHON);
			 else
				  /*exit py thon*/			     
			    {RecoverPikaThoneTaskHandleByName();pks_vm_exit();}
	 }
	 /*À¶ÑÀon - off*/
	 else if (uiListNum == BLUE_LOGO)
	 { 
		  _DEV_CFG *cfg_handle = getCfgFileHandle();
		  BLUE_CONFIG *handle = blueHandle();
	    handle->blueState=!handle->blueState; 
		  cfg_handle->blue_state =handle->blueState;
		  blue_onAndoff(handle->blueState);
		  if(blueHandle()->blueState == 0)
				 _show_write_led((unsigned char*)blue_logo,CalculatedBrightness(BLUE_SET_OFF_COLOR,UI_DEFAULT_BRIGTNESS));
		  else
				 _show_write_led((unsigned char*)blue_logo,CalculatedBrightness(BLUE_SET_ON_COLOR,UI_DEFAULT_BRIGTNESS));
	 }
}
