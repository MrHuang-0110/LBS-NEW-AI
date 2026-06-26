#include "_os.h"
#include "os.h"
#include "motor.h"
#include "PikaVM.h"
extern uint32_t pyTick;
#if 0
void _os_sleep_s(PikaObj *self, pika_float tick)
{ 
  extern bool getRunState(void);
   
	float ms = (tick * 1000.0f);
	while(VMSignal_getCtrl()!= VM_SIGNAL_CTRL_EXIT && ms > 0)
	{
	     pika_GIL_EXIT();
		 vTaskDelay(1);
         pika_GIL_ENTER();
	   ms--;
	}
	 
}
#endif
 
void _os_sleep_s(PikaObj *self, pika_float seconds)
{
 
    uint32_t total_ms = (uint32_t)(seconds * 1000.0f);
    
    if (total_ms == 0) return;
    
    TickType_t start_ticks = xTaskGetTickCount();
    TickType_t target_ticks = start_ticks + pdMS_TO_TICKS(total_ms);
 
    pika_GIL_EXIT();
    
    while (xTaskGetTickCount() < target_ticks) {
        // 检查退出条件
        if (VMSignal_getCtrl() == VM_SIGNAL_CTRL_EXIT) {
            break;
        }
        
        // 计算剩余时间，使用较大的延时块
        TickType_t remaining_ticks = target_ticks - xTaskGetTickCount();
        TickType_t delay_ticks = (remaining_ticks > pdMS_TO_TICKS(50)) ? 
                                pdMS_TO_TICKS(50) : remaining_ticks;
        
        if (delay_ticks > 0) {
           vTaskDelay(delay_ticks);
					 // vTaskDelayUntil(&start_ticks, 10);
        } else {
            break;
        }
    }
    
    pika_GIL_ENTER();
 
}
 

void _os_stop(PikaObj *self, char* name)
{ 
   if(strcmp(name,"stopThisTask") == 0)
   {
      pika_GIL_EXIT();
	    vTaskSuspend(NULL);
	    pika_GIL_ENTER();
   }
   else if(strcmp(name,"stopAllTask") == 0)
   {
      pika_GIL_EXIT();
      SuspendedPikaThoneTaskHandleByName();
	    pika_GIL_ENTER();
   }
   else if(strcmp(name,"stopOtherTask") == 0)
   {
      pika_GIL_EXIT();
      SuspendedOTherPikaThoneTaskHandleByName();
	    pika_GIL_ENTER();
   }
   else if(strcmp(name,"stopAndExit") == 0)
   {
	   RecoverPikaThoneTaskHandleByName();
	   pks_vm_exit();
   }   
}
pika_float _os_voic(PikaObj *self)
{ 
   extern volatile float voic;
	 return voic;
}
pika_float _os_timer(PikaObj *self)
{ 
    return (pika_float)(pyTick/1000.0f);
}
void _os_resetTimer(PikaObj *self)
{ 
	 pika_GIL_EXIT();
   pyTick = 0;
	 pika_GIL_ENTER();
}

pika_float _os_round(PikaObj *self, pika_float var)
{ 
   return round(var);
}
void _os_clear_thread(PikaObj *self)
{ 
   RecoverPikaThoneTaskHandleByName();
	 pks_vm_exit();   
}
