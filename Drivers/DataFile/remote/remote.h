#ifndef __REMOTE_H
#define __REMOTE_H
#include "sys.h"

 
uint8_t *getremotevalue(void);
void refreshRemoteValue(uint8_t *data,uint8_t length);
void clearRemoteValue(void);
void set_remote_linke(void);
void close_remote_linke(void);
uint8_t get_remote_linke(void);

#endif
