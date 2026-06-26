#ifndef __MATCHINESTATE_H
#define __MATCHINESTATE_H
#include "sys.h"
#include "pika_config.h"
 
#define TASK_PRIO_MAT_CHINE_STATE    (PIKA_THREAD_PRIO + 1)
#define TASK_PRIO_USB_DOWNLOAD       (PIKA_THREAD_PRIO + 2)//(PIKA_THREAD_PRIO + 2)  configMAX_PRIORITIES - 2
#define TASK_PRIO_MAST_ENTERY        (PIKA_THREAD_PRIO + 3) 
 

#define EVENT_REFRESH_MATRIX  (1 << 0)   
#define EVENT_SEND_MONITOR    (1 << 1)   
#define EVENT_FIND_PORT_DEV   (1 << 2) 
#define EVENT_USB_FRAM_BYTE   (1 << 3) 
#define EVENT_KEY_ENTERY      (1 << 4) 
//#define EVENT_EXPORT_CSV      (1 << 5)
#define EVENT_BLUE_FRAM_BYTE  (1 << 6)
#define EVENT_RUN_PYTHON      (1 << 7)
//#define EVENT_POWER_DOWN      (1 << 8)
#define EVENT_MEM_REFRESH     (1<<9)
#define EVENT_SENORD_REFRESH  (1<<10)
#define EVENT_BLUE_STATE_REFRESH (1<<11)
#define EVENT_BAT_REFRESH     (1<<12)
#define EVENT_CAMER_MODE      (1<<5)
#define EVENT_PLAYER_KEY_VIOC (1<<16)
void SET_EVENT_GROUP_ISR(EventBits_t event);
void SET_EVENT_GROUP(EventBits_t event);
void NEWAI_CreatePowerOnStartTask(void);
bool getRunState(void);
#endif
