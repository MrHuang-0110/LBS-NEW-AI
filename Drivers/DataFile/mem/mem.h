#ifndef __MEM_H
#define __MEM_H
#include "sys.h"
#include "motion_fx.h"

typedef struct
{
  float x;
  float y;
  float z;
}Axis3f;

typedef struct
{
   int16_t x;
   int16_t y;
   int16_t z;
}Axis3i16;

typedef struct
{
   int x;
   int y;
   int z;
}Axis3i32;

typedef union {
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef struct
{
  unsigned char Lsm6ds3id,Lis2mdlid;
  uint8_t acc_fifo[20*2][6];
  uint8_t gyr_fifo[20*2][6];
  uint8_t timestamp_fifo[20*2][6];
  int16_t data_raw_magnetic[3];
  int16_t gy_raw_data[3],acc_raw_data[3],mg_raw_data[3];
  MFX_output_t mfx_6x;
  MFX_MagCal_output_t mag_data_out;
  axis3bit16_t data_raw_none,
  	           data_raw_angular_rate,
  	           data_raw_acceleration,
  	           data_raw_Timestamp;
  float angular_rate_mdps[3],acceleration_mg[3],magnetic_mG[3];
  uint32_t timestamp,deltatime_1,deltatime_2,deltatime_first;
  float dt;
  float pitch,roll,yaw,X0yaw,X1yaw,sPitch,TargerYaw;
  int dirError;
	uint32_t StartTickState;
	bool is_refresh;
}DEV_MEM;
#define DELATE_TIME		((double)(0.0022))//0.0025
#define ALGO_FREQ  100U /* Algorithm frequency 100Hz */
#define ALGO_PERIOD  (1000U / ALGO_FREQ) /* Algorithm period [ms] */


#define FX_STATE_SIZE (size_t)(2432)

#define ACC_ORIENTATION_X   'n'
#define ACC_ORIENTATION_Y   'e'
#define ACC_ORIENTATION_Z   'd'

#define GYR_ORIENTATION_X   'n'
#define GYR_ORIENTATION_Y   'e'
#define GYR_ORIENTATION_Z   'd'

#define MAG_ORIENTATION_X   's'
#define MAG_ORIENTATION_Y   'e'
#define MAG_ORIENTATION_Z   'd'

#define GBIAS_ACC_TH_SC_6X    (2.0f*0.000765f)
#define GBIAS_GYRO_TH_SC_6X   (2.0f*0.002f)
#define GBIAS_MAG_TH_SC_6X    (2.0f*0.001500f)
#define GBIAS_ACC_TH_SC_9X    (2.0f*0.000765f)
#define GBIAS_GYRO_TH_SC_9X   (2.0f*0.002f)
#define GBIAS_MAG_TH_SC_9X    (2.0f*0.001500f)

#define FROM_MG_TO_G          0.001f
#define FROM_MDPS_TO_DPS      0.001f
#define FROM_MGAUSS_TO_UT50     (0.1f/50.0f)

void memu_fx_init(void);
void lsm6ds3tr_c_motion_fx_determin(void);
bool read_acc_gray(void);
void read_magnetic(void);
DEV_MEM *getMemHandle(void);
void resetyaw(void);
uint16_t get_raw_grayz(void);
void waite_yaw_stab(void);
float get_yaw(void);

float get_continuous_yaw(DEV_MEM *mem, float *last_raw_yaw, float *yaw_offset);
float calculate_angle_difference(float previous_angle, float current_angle);
#endif
