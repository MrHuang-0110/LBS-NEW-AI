#include "elect_sensor.h"
#include "uart.h"
#include "portagree.h"

/* Same placement as the camera/IR modules: plain .ANY RW/ZI (AXI RAM). SRAM2 is
   already tight, and nothing here is touched by DMA. */
static DEV_ELECT_SENSOR dev_elect_sensor[8];

/* Port-scan ticks between re-sends while the echoed state differs from the
   requested one. The scan runs every 50 ms, so 1 means 100 ms (10 Hz), which is
   the rate the vendor protocol explicitly allows for this idempotent command. */
#define ELECT_SENSOR_RETRY_TICKS  1

DEV_ELECT_SENSOR *create_elect_sensor(uint8_t index)
{
	 DEV_ELECT_SENSOR *sensor = &dev_elect_sensor[index];

	 memset(sensor, 0, sizeof(DEV_ELECT_SENSOR));
	 sensor->base.type = DEV_ID_ELECT_SENSOR;
	 sensor->base.devId = index;
	 strncpy(sensor->base.name, "elect_sensor", sizeof(sensor->base.name) - 1);
	 sensor->base.name[sizeof(sensor->base.name) - 1] = '\0';

	 /* Reconnect handshake drops the coil, and desired_state == 0 means the poll
	    below will not try to re-engage it: only an explicit set_state() does. */
	 sensor->state = 0;
	 sensor->desired_state = 0;
	 sensor->retry_tick = 0;

	 return sensor;
}

DEV_ELECT_SENSOR *read_elect_sensor(void *self)
{
	 if(self == NULL)
		   return NULL;
	 return (DEV_ELECT_SENSOR*)self;
}

/* Single place that builds the empty-payload 0xD1/0xD2 command frame.
   MultiUart_SendFrame() memcpy()s from the data pointer, so a valid placeholder
   address is always passed - never NULL - even though len is 0. */
void elect_sensor_send_state(uint8_t port, uint8_t state)
{
	 uint8_t empty_payload = 0;

	 if(port > 7)
		   return;
	 if(state > 1)
		   return;

	 MultiUart_SendFrame((UartDevice_t)port,
	                     &empty_payload,
	                     0,
	                     DEV_ID_ELECT_SENSOR,
	                     (state == 1) ? ELECT_SENSOR_CMD_ON : ELECT_SENSOR_CMD_OFF,
	                     10, 250);
}

/* 0xED uplink: exactly one state byte. base.data_len was set from the frame
   length by scan_agreement_data_port_dev() before this frame was queued, so a
   truncated/oversized frame or an out-of-range state keeps the previous valid
   value instead of corrupting the reported state. */
void refsh_elect_sensor(DEV_ELECT_SENSOR *sensor, uint8_t *data)
{
	 if(sensor == NULL || data == NULL)
		   return;
	 if(sensor->base.data_len != 1)
		   return;                    /* malformed frame: keep previous value */
	 if(data[0] > 1)
		   return;                    /* protocol only defines 0 = released, 1 = engaged */

	 sensor->state = data[0];         /* command state echoed by the sensor board */
}

/* Called from the 50 ms port scan while the device is bound.
   A mismatch between the echoed state and the requested state means either the
   command frame never arrived (devTxQueue is depth 1 with a 10 ms timeout, so a
   single frame can be dropped) or the device re-handshook/reset and forced the
   coil off. Both are recovered by re-sending the requested state, which the
   vendor protocol requires and which is idempotent. */
void elect_sensor_poll(DEV_ELECT_SENSOR *sensor)
{
	 if(sensor == NULL)
		   return;

	 if(sensor->state == sensor->desired_state)
	 {
		  sensor->retry_tick = 0;     /* already in sync: nothing to re-send */
		  return;
	 }

	 if(sensor->retry_tick > 0)
	 {
		  sensor->retry_tick--;
		  return;
	 }

	 sensor->retry_tick = ELECT_SENSOR_RETRY_TICKS;
	 elect_sensor_send_state((uint8_t)sensor->base.devId, sensor->desired_state);
}

void free_dev_elect_sensor(uint8_t port)
{
	 DEV_ELECT_SENSOR *sensor = read_elect_sensor((SensorBase *)getDevBase(port));
	 if(sensor != NULL)
	 {
		  memset(sensor, 0, sizeof(DEV_ELECT_SENSOR));
	 }
}
