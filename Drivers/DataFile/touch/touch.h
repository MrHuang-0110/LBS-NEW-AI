#ifndef __TOUCH_H
#define __TOUCH_H
#include "sys.h"
 
#define DEV_ID_TOUCH          0xA4

typedef struct
{ 
	 SensorBase base;
   int touchState;
}DEV_TOUCH;


DEV_TOUCH *read_touch(void *self);
void refsh_touch(DEV_TOUCH* mt, uint8_t* data);
DEV_TOUCH *create_touch(uint8_t index);
void freeTouch(uint8_t port);
#endif
