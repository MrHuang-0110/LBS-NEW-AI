#ifndef __GRAYV2_H
#define __GRAYV2_H
#include "sys.h"
#include "motor.h"
#define DEV_ID_GRAY_V2           0xB0


typedef enum{ 
  T_WAY,
	LEFT_WAY,
	RIGHT_WAY,
	MIDDLE_WAY,
	ONE_WAY,
	TWO_WAY,
	TREE_WAY,
	FOURE_WAY,
	FIVE_WAY,
	SIX_WAY,
	SEVEN_WAY,
	WAY_MAX
}WAY_TYPE;

typedef struct{
	 SensorBase base;
   int L_Value[7];
   int L_State[7];
	 int Threshold[7];
	 float kp,kd;
   int version;
	 uint8_t lindex,mindex,rindex;
	 bool is_resh_flag,is_calibrate;
}DEV_GRAYV2;

 

DEV_GRAYV2 *read_gray_v2(void *self);
void refsh_gray_v2(DEV_GRAYV2* mt, uint8_t* data);
DEV_GRAYV2 *create_gray_v2(uint8_t index);
void free_gray_v2(uint8_t id);

uint8_t getWayType(int port,WAY_TYPE type);
 void grayv2_find_line(DEV_GRAYV2 *grayv2,_USER_DOUBLE_MOTOR *doubleMotor,DEV_MOTOR *master,DEV_MOTOR *slave,float power);
#endif
