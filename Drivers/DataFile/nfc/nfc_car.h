#ifndef __NFC_CAR_H
#define __NFC_CAR_H
#include "sys.h"

#define DEV_ID_NFC 0xB2

typedef struct
{
	SensorBase base;
  int car_id,version;
}DEV_NFC;

void refsh_nfc(void* self, uint8_t* data);
DEV_NFC *read_nfc(void *self);
DEV_NFC *create_nfc_car(uint8_t index);
void free_dev_nfc(uint8_t port);
#endif
