#ifndef __CAMER_H
#define __CAMER_H
#include "sys.h"

#define DEV_ID_CAMER 0xA7
 
typedef enum{ 
  CAMER_MENU_TYPE = 0x01,
	CAMER_MODE_TYPE = 0x02,
	CAMER_FACE_TYPE = 0x03,
	CAMER_LABE_TYPE = 0x04,
	CAMER_OBJECT_TYPE = 0x05,
	CAMER_COLOR_TYPE = 0x06,
	CAMER_WAY_TYPE = 0x07,
	CAMER_GESTURE_TYPE = 0x10,
	CAMER_BODY_TYPE = 0x11,
	CAMER_OBJECT_BODY_TYPE = 0x12,
	CAMER_PHOTO_TYPE = 0x13
}CAMER_MODE;
 
 
typedef struct
{
	SensorBase base;
	CAMER_MODE mode;
	uint8_t data[40];
}DEV_CAMER;

DEV_CAMER *create_camer(uint8_t index);
DEV_CAMER *read_camer(void *self);
void refsh_camer(DEV_CAMER* mt, uint8_t index,uint8_t* data);

bool is_camer_base(uint8_t id);
void set_hw_camer_mode(void);
void free_camer(uint8_t id);
#endif

