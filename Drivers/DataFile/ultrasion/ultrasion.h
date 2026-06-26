#ifndef __ULTRASION_H
#define __ULTRASION_H
#include "sys.h"

#define DEV_ID_ULTRASION      0xA3

typedef struct
{ 
	 SensorBase base;
   float  cm;
	 bool is_resh_flag;
}DEV_ULTRASION;


DEV_ULTRASION *read_ultrasion(void *self);
void refsh_ultrasion(DEV_ULTRASION* mt, uint8_t* data);
DEV_ULTRASION *create_ultrasion(uint8_t index);
void free_ultrasion(uint8_t port);

#endif
