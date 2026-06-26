#ifndef __MESSAGE_H
#define __MESSAGE_H
#include "sys.h"

 
#define MAX_EVENT_ID 32

typedef struct
{
  uint8_t eventId[MAX_EVENT_ID];
  uint8_t setEventId[MAX_EVENT_ID];
  uint8_t setEventIdWaite[MAX_EVENT_ID];
}_MEASSGE_EVENT_;


#endif
