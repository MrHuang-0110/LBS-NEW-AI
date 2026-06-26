#ifndef __DATASTRUCT_H
#define __DATASTRUCT_H
#include "sys.h"
#include "stdbool.h"
#if 0
typedef struct{
	uint8_t portNumber;
  int dataError;
	int crcError;
	int     count;
	uint8_t data[256];
}USART_DEBUG;
#endif
 typedef struct {
   int  type,findIndex,devId;	 
	 uint8_t data[64];
   char name[16];
   void (*setParam)(void* self, uint8_t *data);
}SensorBase;
 
typedef struct
{ 
	uint16_t portTimeOutTick;
	uint8_t _LinkeObjDev,_LastLinkeObjDev;
	SensorBase* sensors;   
	
	uint8_t ultrasionTick;
 
}__PORT;  
 
bool identify_and_bind(__PORT *manager,uint8_t devId,uint8_t index);
void set_sensor_parameter(SensorBase* sensor,uint8_t *param);
void setAck(void* self, uint8_t *data);
#endif
