#include "_ir.h"
#include "ir.h"
#include "uart.h"
#include "portagree.h"

/* _ir.set_rgb(port, state)
 *   state: 0=off 1=red 2=green 3=blue  -> downlink 0xD1 + 1 byte payload.
 *
 * Command-state semantics (see the protocol doc): the device stores the value
 * and echoes it in the 10 ms 0xED uplink. It is NOT proof that the battery
 * powered receiver board actually changed colour. */
void _ir_set_rgb(PikaObj *self, int port, int state)
{
	 DEV_IR *ir;

	 if(port < 0 || port > 7)
		   return;

	 ir = read_ir((SensorBase *)getDevBase(port));
	 if(ir == NULL || ir->base.type != DEV_ID_IR)
		   return;                    /* this port has no bound IR module */

	 if(state < 0 || state > 3)
		   return;                    /* protocol: >=4 is silently ignored by the device */

	 uint8_t payload = (uint8_t)state;

	 MultiUart_SendFrame((UartDevice_t)port,
	                     &payload,
	                     1,
	                     IR_WIRE_OBJECT_ID,
	                     IR_CMD_SET_COLOR,
	                     10, 250);
}
