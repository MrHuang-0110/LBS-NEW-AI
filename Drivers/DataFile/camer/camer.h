#ifndef __CAMER_H
#define __CAMER_H
#include "sys.h"

#define DEV_ID_CAMER 0xA7
 
typedef enum{ 
  CAMER_MENU_TYPE = 1,
	CAMER_MODE_TYPE,
	CAMER_FACE_TYPE,
	CAMER_LABE_TYPE,
	CAMER_OBJECT_TYPE,
	CAMER_COLOR_TYPE,
	CAMER_WAY_TYPE,
	CAMER_GESTURE_TYPE,
	CAMER_BODY_TYPE,
	CAMER_OBJECT_BODY_TYPE,
	CAMER_PHOTO_TYPE
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

