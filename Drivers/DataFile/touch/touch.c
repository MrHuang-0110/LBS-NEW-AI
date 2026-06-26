#include "touch.h"
//#include "malloc.h"
#include "portagree.h"

static DEV_TOUCH __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_touch[8];

DEV_TOUCH *read_touch(void *self)
{
   DEV_TOUCH *mt = (DEV_TOUCH*)self;
	 return mt;
}
void refsh_touch(DEV_TOUCH* mt, uint8_t* data)
{ 
			  mt->touchState = *data;	  
}

DEV_TOUCH *create_touch(uint8_t index)
{ 
 				DEV_TOUCH *touch = &dev_touch[index];
				memset(touch,0,sizeof(DEV_TOUCH));
				touch->base.type = DEV_ID_TOUCH;
				memset(touch->base.name,0,sizeof(touch->base.name));
				strcpy(touch->base.name,"touch"); 
				touch->touchState = 0;
 
				return touch;		  
}

void freeTouch(uint8_t port)
{ 
    DEV_TOUCH *touch = read_touch((SensorBase *)getDevBase(port));	
   if(touch!=NULL)
   {
	    memset(touch,0,sizeof(DEV_TOUCH));	
	 }		 
}
