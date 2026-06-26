#ifndef __EQUIPMENT_H
#define __EQUIPMENT_H
#include "sys.h"
#include "portagree.h"
 
 
 
typedef struct {
    uint8_t device_type;
    uint8_t device_id;
    uint16_t cmd_size;     
} CommandHeader;

typedef struct
{ 
  TimerHandle_t timer;
	void *cmd;
}TimerCommandPair;
 
typedef struct {
    CommandHeader header;
    uint8_t sample_rate;
    bool calibrate;
} SensorCommandData;

TimerCommandPair* create_timer_with_cmd(void *cmd_data, uint32_t delay_ms);
void timer_callback_wrapper(TimerHandle_t xTimer);
 
#endif

