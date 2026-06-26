#ifndef __BLUE_H
#define __BLUE_H
#include "sys.h"

typedef struct
{ 
  uint8_t blueState,atState;
	uint8_t atData[32];
	char BLUE_MAC[32];
	char *blueMonitorBuffer;
}BLUE_CONFIG;

BLUE_CONFIG *blueHandle(void);
bool is_valid_at_command(const char *data, size_t len);
void is_get_at_data(uint8_t *atData,uint8_t *pData,size_t len);
void blue_onAndoff(uint8_t on_off);
void blue_init(void);
void close_blueAndFeedIWDG(void);
#endif
