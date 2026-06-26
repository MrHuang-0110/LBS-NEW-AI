#include "gray.h"
//#include "malloc.h"
#include "portagree.h"

static DEV_GRAY __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_gray[8];

DEV_GRAY *read_gray(void *self)
{ 
	 DEV_GRAY *mt = (DEV_GRAY*)self;
	 return mt;
}

void refsh_gray(DEV_GRAY* mt, uint8_t* data)
{ 
 
  sscanf((const char*)data,"%d/%d/%d/%d/%d/%d/%d/%d/%d",&mt->L1,
																												&mt->L2,
																												&mt->R1,
																												&mt->R2,
																												&mt->L1State,
																												&mt->L2State,
																												&mt->R1State,
																												&mt->R2State,
																												&mt->version);	
	mt->is_resh_flag = true;
}

DEV_GRAY *create_gray(uint8_t index)
{ 
 	 DEV_GRAY *gray = &dev_gray[index];
	 memset(gray,0,sizeof(DEV_GRAY));
	 gray->base.type = DEV_ID_GRAY;
	 memset(gray->base.name,0,sizeof(gray->base.name));
	 strcpy(gray->base.name,"gray"); 
 
	 return gray;	  
}

void free_gray(uint8_t id)
{ 
	 DEV_GRAY *gray = read_gray((SensorBase *)getDevBase(id));
   if(gray!=NULL)
	 {
	    memset(gray,0,sizeof(DEV_GRAY));
	 }
}
