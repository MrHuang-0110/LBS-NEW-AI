#include "_elect_sensor.h"
#include "elect_sensor.h"
#include "portagree.h"

/* _elect_sensor.set_state(port, state)
 *   state 1 -> downlink 0xD1 with an EMPTY payload (engage the electromagnet)
 *   state 0 -> downlink 0xD2 with an EMPTY payload (release it)
 *
 * The requested state is stored on the device object and sent immediately, so the
 * command takes effect within ~1 ms. elect_sensor_poll() (driven by the existing
 * 50 ms port scan) then keeps it applied, because the device sends no ACK: a
 * command frame dropped by the depth-1 TX queue, or a re-handshake/device reset
 * (which always forces the outputs off), would otherwise leave the coil in the
 * wrong state forever. The vendor protocol requires the host to re-send in
 * exactly those cases, and the command is idempotent.
 *
 * Out-of-range ports/states, and ports without a bound 0xE0 device, are silently
 * ignored - same style as the other device APIs. */
void _elect_sensor_set_state(PikaObj *self, int port, int state)
{
	 DEV_ELECT_SENSOR *sensor;

	 if(port < 0 || port > 7)
		   return;

	 sensor = read_elect_sensor((SensorBase *)getDevBase(port));
	 if(sensor == NULL || sensor->base.type != DEV_ID_ELECT_SENSOR)
		   return;                    /* this port has no bound electromagnetic sensor */

	 if(state != 0 && state != 1)
		   return;                    /* protocol only defines 0 = release, 1 = engage */

	 sensor->desired_state = (uint8_t)state;
	 sensor->retry_tick = 0;          /* let the next port scan re-send immediately */
	 elect_sensor_send_state((uint8_t)port, (uint8_t)state);
}
