#include "_touch.h"
#include "touch.h"
#include "PikaVM.h"
#include "portagree.h"
 
 
pika_float _touch_state(PikaObj *self, int port)
{ 
  DEV_TOUCH *touch = read_touch((SensorBase *)getDevBase(port));
	
	if(touch == NULL)
		  return 0;
	
	return (float)touch->touchState;
}
