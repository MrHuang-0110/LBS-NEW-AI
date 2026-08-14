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
	uint8_t n_targets;   /* 本帧目标数量 N (0~25),由帧长度/10 得出 */
	uint8_t data[256];   /* 标签识别动态帧:N*10 字节,N<=25 */
	/* Name frame (0x0E) cache: payload = src_type(1B) + sum(id,name_len,name_utf8),<=250B */
	uint8_t name_src_type; /* owning mode type code (0x04/0x05/0x10/0x12) */
	uint8_t name_len;      /* name-frame payload total length (incl. src_type) */
	uint8_t name_data[250];/* raw name-frame payload */
}DEV_CAMER;

DEV_CAMER *create_camer(uint8_t index);
DEV_CAMER *read_camer(void *self);
void refsh_camer(DEV_CAMER* mt, uint8_t index,uint8_t* data);
void setCamerName(void* self, uint8_t *data);   /* name frame 0x0E direct cache, no mode update */

/* Lookup name in name-frame cache by id; returns pointer into name_data
   (NOT NUL-terminated), *out_len = name byte length; NULL if not found. */
const char *camer_find_name(const DEV_CAMER *camer, uint8_t id, uint8_t *out_len);

bool is_camer_base(uint8_t id);
void set_hw_camer_mode(void);
void free_camer(uint8_t id);
#endif

