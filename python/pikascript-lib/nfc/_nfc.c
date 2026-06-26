#include "_nfc.h"
#include "nfc_car.h"
#include "portagree.h"
pika_float _nfc_read_car(PikaObj *self, int port)
{ 
  DEV_NFC *nfc = read_nfc((SensorBase *)getDevBase(port));
	
	if(nfc==NULL)return 0;
	
	return nfc->car_id;
}
