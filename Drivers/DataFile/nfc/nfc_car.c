#include "nfc_car.h"
//#include "malloc.h"
#include "portagree.h"
static DEV_NFC __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_nfc[8];
void refsh_nfc(void* self, uint8_t* data)
{ 
	 DEV_NFC *mt = (DEV_NFC*)self;
	 sscanf((const char*)data,"%d/%d",&mt->car_id,&mt->version);
}
DEV_NFC *create_nfc_car(uint8_t index)
{ 
	 DEV_NFC *nfc = &dev_nfc[index];
	
	 memset(nfc,0,sizeof(DEV_NFC));
	 nfc->base.type = DEV_ID_NFC;
	 memset(nfc->base.name,0,sizeof(nfc->base.name));
	 strcpy(nfc->base.name,"nfc"); 
  
	 return nfc;	  	
}
DEV_NFC *read_nfc(void *self)
{ 
   DEV_NFC *mt = (DEV_NFC*)self;
	 return mt; 
}
void free_dev_nfc(uint8_t port)
{ 
    DEV_NFC *nfc = read_nfc((SensorBase *)getDevBase(port));	
   if(nfc!=NULL)
   {
	     memset(nfc,0,sizeof(DEV_NFC));
	 }		    
}
