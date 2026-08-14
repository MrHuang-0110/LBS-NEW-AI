#include "camer.h"
#include "matchineState.h"
#include "uart.h"
#include "portagree.h"
//#include "malloc.h"
#include "motor.h"

static DEV_CAMER dev_camer[8];
 

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
    (void)data;                              /* 数据已由 setCamerAck 直通写入 */
    mt->mode = (CAMER_MODE)index;
}

/* Name frame (0x0E) direct cache: payload = src_type(1B) + sum(id,name_len,name_utf8).
   No queue, no mode update (0x0E is a name frame, not a mode; avoids refsh_camer
   writing mode=0x0E). */
void setCamerName(void* self, uint8_t *data)
{
    SensorBase *base = (SensorBase*)self;
    DEV_CAMER *camer = (DEV_CAMER*)self;

    size_t len = base->data_len;
    if (len == 0)
    {
        camer->name_len = 0;
        camer->name_src_type = 0;
        return;
    }
    if (len > sizeof(camer->name_data))
        len = sizeof(camer->name_data);

    memset(camer->name_data, 0, sizeof(camer->name_data));
    memcpy(camer->name_data, data, len);
    camer->name_len = (uint8_t)len;
    camer->name_src_type = data[0];          /* src_type = owning mode type code */
}

/* Lookup name in name-frame cache by id (learned slot id / tag code / QR seq).
   Returns pointer into name_data (NOT NUL-terminated, frame stores raw items),
   *out_len gets the name byte length; NULL if not found. */
const char *camer_find_name(const DEV_CAMER *camer, uint8_t id, uint8_t *out_len)
{
    uint8_t n = camer->name_len;
    if (n < 1) return NULL;
    uint16_t pos = 1;                       /* skip src_type */
    while ((uint16_t)pos + 2 <= n)
    {
        uint8_t nid  = camer->name_data[pos];
        uint8_t nlen = camer->name_data[pos + 1];
        if ((uint16_t)pos + 2 + nlen > n) break;   /* bounds guard */
        if (nid == id)
        {
            if (out_len) *out_len = nlen;
            return (const char *)&camer->name_data[pos + 2];
        }
        pos += 2 + (uint16_t)nlen;
    }
    return NULL;
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

