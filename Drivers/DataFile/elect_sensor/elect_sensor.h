#ifndef ELECT_SENSOR_H_
#define ELECT_SENSOR_H_
#include "dataStruct.h"

/* ELECT_SENSOR (electromagnetic engagement / electromagnet sensor) - host-side
 * device module.
 *
 * Wire protocol: LBS-NEW-AI-SENSORD/Doc/ELECTROMAGNETIC_SENSOR_protocol.md
 *   - handshake 0x09 "Please Link" -> "Play Aplication" (sic), ObjectID 0xE0.
 *     The handshake ObjectID is used directly as the host DEV_ID_* (no internal
 *     ID mapping, unlike IR_REMOTE).
 *   - uplink   0xED every ~6 ms, payload = exactly 1 byte command state:
 *     0 = released, 1 = engaged. A frame whose length is not 1, or whose state
 *     byte is outside 0/1, is ignored and the last valid state is kept.
 *   - downlink 0xD1 with an EMPTY payload = engage, 0xD2 with an EMPTY payload
 *     = release. The device requires the command to be exactly 7 bytes
 *     (len == 0) delivered as ONE contiguous burst - it delimits frames with the
 *     USART IDLE interrupt, and its 0xD1/0xD2 handlers additionally require
 *     rx_data->len >= 7.
 *
 * The reported state is the command state the device echoes back. It is NOT a
 * physical coil/contact feedback.
 *
 * DELIVERY MODEL (why the command is re-sent):
 *   The device sends no ACK and the host's TX queue is depth 1 with a 10 ms
 *   timeout, so a single command frame can be dropped silently. The device also
 *   forces both outputs off on every handshake, so a reconnect/reset/re-handshake
 *   drops an engaged coil. The vendor protocol therefore requires the host to
 *   (a) re-send when 0xED does not echo the requested state, and (b) restore the
 *   requested state after a re-handshake; re-sending is idempotent and 10 Hz is
 *   explicitly allowed. elect_sensor_poll() implements both.
 *
 * A full unplug/replug frees the port, and create_elect_sensor() resets
 * desired_state to 0, so a physical reconnect never re-energises the coil on its
 * own - a user program has to call set_state(port, 1) again. */
#define DEV_ID_ELECT_SENSOR     0xE0

#define ELECT_SENSOR_CMD_ON     0xD1
#define ELECT_SENSOR_CMD_OFF    0xD2

typedef struct
{
	 SensorBase base;        /* base.data[] keeps the raw 0xED payload, base.data_len its length */
	 uint8_t state;          /* last state the device echoed (0 = released, 1 = engaged) */
	 uint8_t desired_state;  /* state the user program asked for; re-sent until it is echoed */
	 uint8_t retry_tick;     /* port-scan ticks still to wait before the next re-send */
}DEV_ELECT_SENSOR;

DEV_ELECT_SENSOR *create_elect_sensor(uint8_t index);
DEV_ELECT_SENSOR *read_elect_sensor(void *self);
void refsh_elect_sensor(DEV_ELECT_SENSOR *sensor, uint8_t *data);
void free_dev_elect_sensor(uint8_t port);
void elect_sensor_send_state(uint8_t port, uint8_t state);
void elect_sensor_poll(DEV_ELECT_SENSOR *sensor);
#endif
