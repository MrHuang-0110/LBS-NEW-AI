#include "ir.h"
#include "uart.h"
#include "portagree.h"

/* Same placement as the camera module: plain .ANY RW/ZI (AXI RAM). SRAM2 is
   already tight, and nothing here is touched by DMA. */
static DEV_IR dev_ir[8];

DEV_IR *create_ir(uint8_t index)
{
	 DEV_IR *ir = &dev_ir[index];

	 memset(ir, 0, sizeof(DEV_IR));
	 ir->base.type = DEV_ID_IR;
	 ir->base.devId = index;
	 strncpy(ir->base.name, "ir", sizeof(ir->base.name) - 1);
	 ir->base.name[sizeof(ir->base.name) - 1] = '\0';

	 ir->state = 0;

	 return ir;
}

DEV_IR *read_ir(void *self)
{
	 if(self == NULL)
		   return NULL;
	 return (DEV_IR*)self;
}

/* 0xED uplink: packed {state, bat} - deliberately NO version field, unlike the
   motor/color/gray packets. Only the command state is kept (the battery byte is
   part of the frame but unused). base.data_len was set from the frame length by
   scan_agreement_data_port_dev() before this frame was queued. */
void refsh_ir(DEV_IR *ir, uint8_t *data)
{
	 if(ir == NULL || data == NULL)
		   return;
	 if(ir->base.data_len < 1)
		   return;                    /* short/malformed frame: keep previous value */

	 ir->state = data[0];             /* command state echoed by the emitter board */
}

void free_dev_ir(uint8_t port)
{
	 DEV_IR *ir = read_ir((SensorBase *)getDevBase(port));
	 if(ir != NULL)
	 {
		  memset(ir, 0, sizeof(DEV_IR));
	 }
}
