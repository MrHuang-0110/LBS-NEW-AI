#ifndef __GRAY_H
#define __GRAY_H
#include "sys.h"
#define DEV_ID_GRAY           0xA9

 

typedef struct{
	 SensorBase base;
   int L1,L2,R1,R2;
   int L1State,L2State,R1State,R2State;
   int version;
	 bool is_resh_flag;
}DEV_GRAY;

 

DEV_GRAY *read_gray(void *self);
void refsh_gray(DEV_GRAY* mt, uint8_t* data);
DEV_GRAY *create_gray(uint8_t index);
void free_gray(uint8_t id);
#endif
