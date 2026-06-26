#include "camer.h"
#include "matchineState.h"
#include "uart.h"
#include "portagree.h"
//#include "malloc.h"
#include "motor.h"

static DEV_CAMER __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_camer[8];
 

DEV_CAMER *create_camer(uint8_t index)
{
 
    DEV_CAMER *camer = &dev_camer[index];
     memset(camer,0,sizeof(DEV_CAMER));
    camer->base.type = DEV_ID_CAMER;
 
    strncpy(camer->base.name, "camer", sizeof(camer->base.name) - 1);
    camer->base.name[sizeof(camer->base.name) - 1] = '\0';
 
//    camer->mode = 0;
 
    SET_EVENT_GROUP_ISR(EVENT_CAMER_MODE);
 
    return camer;
}

DEV_CAMER *read_camer(void *self)
{ 
   DEV_CAMER *mt = (DEV_CAMER*)self;
	 return mt; 
}

void refsh_camer(DEV_CAMER* mt, uint8_t index,uint8_t* data)
{ 
  
    mt->mode = (CAMER_MODE)index;
    switch(index)
    {
			case CAMER_MENU_TYPE:
			case CAMER_MODE_TYPE:
			case CAMER_FACE_TYPE:
	    case CAMER_LABE_TYPE:
	    case CAMER_OBJECT_TYPE:
	    case CAMER_COLOR_TYPE:
	    case CAMER_WAY_TYPE:
	    case CAMER_GESTURE_TYPE:
	    case CAMER_BODY_TYPE:
	    case CAMER_OBJECT_BODY_TYPE:
	    case CAMER_PHOTO_TYPE:	
				memset(mt->data,0,sizeof(mt->data));
			  memcpy(mt->data,data,sizeof(mt->data)); 
			break;
    }
 
}

bool is_camer_base(uint8_t id)
{ 
	return (read_camer((SensorBase *)getDevBase(id))!=NULL?false:true);
}

void free_camer(uint8_t id)
{
   DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(id));
	 if(camer!=NULL)
	 {
	    memset(camer,0,sizeof(DEV_CAMER));
	 }
}

void set_hw_camer_mode(void)
{ 
   for(uint8_t i = 0; i < 8; i++) {
        if(is_camer_base(i))
            continue;  

        DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(i));
				if(camer == NULL)return;
				
        uint8_t data[4];
        memset(data,0,sizeof(data));
        
        data[0] = 255;
        data[1] = 255;
        data[2] = 255;
        data[3] = 50;
				
//        MultiUart_SendFrame((UartDevice_t)i,
//													   data,
//														 4,
//														 DEV_ID_CAMER,
//														 FIND_BLOCK_MODE,
//														 10,
//														 250);
				#if 0
        sensord_send_frame((UartDevice_t)i,
						   (uint8_t)FIND_BLOCK_MODE,
						   DEV_ID_CAMER,	
				       data,
						   4);
				#endif
    }

    
}

