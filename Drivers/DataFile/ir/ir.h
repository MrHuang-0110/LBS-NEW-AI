#ifndef IR_H_
#define IR_H_
#include "dataStruct.h"

/* IR_REMOTE (infrared emitter / receiver pair) - host-side device module.
 *
 * Wire protocol: LBS-NEW-AI-SENSORD/Doc/IR_REMOTE_protocol.md
 *   [5A][ObjectID][SourceID][len][typeIndex][payload...][crc][A5]
 *   - uplink   0xED, payload = packed {state(1B), bat(1B)} - NO version field.
 *     Only state is used here; the receiver battery byte is ignored.
 *   - downlink 0xD1 + 1 byte state: 0=off 1=red 2=green 3=blue
 *
 * ID NOTE: the device firmware reports ObjectID 0xA3, the same value as
 * DEV_ID_ULTRASION. Ultrasonic devices are only ever bound from the ADC voltage
 * window (see FindProtDev), never from a handshake, so port_linke() maps the
 * wire 0xA3 to this host-internal tag and the ultrasonic path stays untouched. */
#define IR_WIRE_OBJECT_ID   0xA3
#define DEV_ID_IR           0xB3

#define IR_CMD_SET_COLOR    0xD1

typedef struct
{
	 SensorBase base;      /* base.data[] keeps the raw 2-byte 0xED payload */
	 uint8_t state;        /* last commanded state 0..3 (command state, NOT an ack) */
}DEV_IR;

DEV_IR *create_ir(uint8_t index);
DEV_IR *read_ir(void *self);
void refsh_ir(DEV_IR *ir, uint8_t *data);
void free_dev_ir(uint8_t port);
#endif
