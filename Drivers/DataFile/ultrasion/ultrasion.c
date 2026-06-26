#include "ultrasion.h"
//#include "malloc.h"
#include "portagree.h"
static DEV_ULTRASION __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_ultrasion[8];
DEV_ULTRASION *read_ultrasion(void *self)
{
   DEV_ULTRASION *mt = (DEV_ULTRASION*)self;
	 return mt;
}
void refsh_ultrasion(DEV_ULTRASION* mt, uint8_t* data)
{ 
			  uint32_t i_cm = (data[0]<<16)+(data[1]<<8)+data[2];		 
			 	mt->cm= (float)(i_cm/1000.0f);
			 	mt->cm/=10.0f;
			  mt->cm= mt->cm > 100.0f?100.0f:mt->cm;	 		
				mt->is_resh_flag = true;
}

DEV_ULTRASION *create_ultrasion(uint8_t index)
{ 
 				DEV_ULTRASION *ultrasion = &dev_ultrasion[index];
				memset(ultrasion,0,sizeof(DEV_ULTRASION));
				ultrasion->base.type = DEV_ID_ULTRASION;
				memset(ultrasion->base.name,0,sizeof(ultrasion->base.name));
				strcpy(ultrasion->base.name,"ultrasion"); 
				ultrasion->cm = 0.0f;
 
				return ultrasion;		  
}

void free_ultrasion(uint8_t port)
{ 
     DEV_ULTRASION *ultrasion = read_ultrasion((SensorBase *)getDevBase(port));	
   if(ultrasion!=NULL)
   {
	    memset(ultrasion,0,sizeof(DEV_ULTRASION));	
	 }		  
}
