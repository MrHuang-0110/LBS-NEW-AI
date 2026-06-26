#include "_message.h"
#include "message.h"
#include "PikaVM.h"

static _MEASSGE_EVENT_ m_event;
extern bool getRunState(void);

void _message_FlashMessageBox(PikaObj *self, pika_float num)
{ 
    if((uint32_t)num >= MAX_EVENT_ID)
				return;
    uint32_t id = (uint32_t)num;
    m_event.setEventIdWaite[id] = 1;
}
void _message_messageInit(PikaObj *self)
{ 
  ;
}
void _message_setMessageBoxWaite(PikaObj *self, pika_float num)
{ 

    if((uint32_t)num >= MAX_EVENT_ID)
					return;


    uint32_t id = (uint32_t)num;
    m_event.setEventId[id] = 1;
		
		while(VMSignal_getCtrl() != 1)
		{
			if(m_event.setEventIdWaite[id] == 1)
					break;

		  pika_GIL_EXIT();
		  vTaskDelay(10);
		  pika_GIL_ENTER();
	  }
		
	m_event.setEventIdWaite[id] = 0;
}
void _message_setMessageBox(PikaObj *self, pika_float num)
{ 
   if((uint32_t)num >= MAX_EVENT_ID)
	   return;

   uint32_t id = (uint32_t)num;
   m_event.setEventId[id] = 1;
	 
}
void _message_waiteMessageBox(PikaObj *self, pika_float num)
{ 
		if((uint32_t)num >= MAX_EVENT_ID)
				return;

   uint32_t id = (uint32_t)num;
   while(VMSignal_getCtrl() != 1)
   {
		if(m_event.setEventId[id] == 1)
			  break;

		pika_GIL_EXIT();
		vTaskDelay(10);
		pika_GIL_ENTER();

   }
   m_event.setEventId[id] = 0;
}
