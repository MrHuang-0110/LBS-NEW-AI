#ifndef __MOTOR_H
#define __MOTOR_H
#include "sys.h"
#include "filter.h"
#include "pid_control.h"
#include "uart.h"

#define PI 3.14159265358979323846f

#define MAX_PWM  100
 
#define DEV_ID_BIG_MOTOR      0xA1
#define DEV_ID_SMALL_Motor    0xA6
 
/*big motor cfg*/
#define BIG_MOTOR_MAX_SPEED             255
#define BIG_MOTOR_MIN_SPEED             32
#define BIG_MOTOR_ENCTORDINFO           90

/*small motor cfg*/
#define SMALL_MOTOR_MAX_SPEED           355
#define SMALL_MOTOR_MIN_SPEED           102
#define SMALL_MOTOR_ENCTORDINFO         90
 
#define MAX_OUTPUT_PWM 100

enum{
 MOTOR_IDLE = 0,
 MOTOR_STOP_SILD,
 MOTOR_STOP_BREAK,
 MOTOR_STOP,
 MOTOR_SPEED,
 MOTOR_SPEED_POS,
 MOTOR_PWM
};


typedef struct{ 
	uint8_t stop_mode,temap_stop;
	int temap_uSpeed,last_temap_uSpeed,uSpeed,pwm;
}_USER_MOTOR;

typedef struct{
  uint8_t id[2],doubleMotor_StopModule;
	int  uSpeed[2];
	bool is_motor_handle;
	
	PIController 			 yaw_pid;
	PIController pid_find_line;
}_USER_DOUBLE_MOTOR;

typedef struct{
	int32_t last_pos;          
  uint32_t last_time;         
  float speed_rpm;           
  float speed_filtered;      	
  int32_t position_buffer[4]; 
  uint32_t time_buffer[4];    
  uint8_t buffer_index;       
	int     speed;
  int     angle;
  float   circle;
	int     current_pos,current_encords;
	int     version;		
	uint8_t dev_id;
}MOTOR_RUN_INFO;

typedef struct{
  int32_t enctordinfo;
	float max_speed,min_speed,v50Speed;
}MOTOR_HW_INFO;

typedef struct{
	 uint8_t current_mode;
	
   PositionController control_position;
   PIController   		control_V50_Speed;
	
   int32_t target_position;     
   int32_t target_velocity;     
   int32_t max_velocity;         
 
   int32_t current_position;    
   int32_t current_velocity;    
  
   int16_t pwm_output;             
 
   bool enable;                 
   bool position_reached;        
   uint32_t position_tolerance; 
	
}MOTOR_CONTROL;

typedef struct
{
	 SensorBase base;  
   _USER_MOTOR    motor_user_info;  
	 MOTOR_RUN_INFO motor_run_info;
	 MOTOR_CONTROL control;
	 float control_dt;
}DEV_MOTOR;

#define DUTY_TO_PWM_SIMPLE(duty) \
((duty) > 0 ? (duty - 1) : \
(duty) < 0 ? (duty + 1) : 0)   

#define ifTypenotNull(x) \
(x!=CMD_IDLE)?false:true
#define RATE_LIMITATION(x, min, max) \
(((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
 
DEV_MOTOR *read_motor(void *self);
DEV_MOTOR *get_motor_base(uint8_t id);
DEV_MOTOR *create_small_motor(uint8_t id);
DEV_MOTOR *create_big_motor(uint8_t id);

_USER_DOUBLE_MOTOR *getDoubleMotorHandle(void);
void refsh_motor(DEV_MOTOR *motor,uint8_t *data);

 
/*motor_controll hook*/
void  motor_hw_stop(uint8_t id,uint8_t mode);
void  motor_hw_set_pwm(uint8_t id,int pwm);		
int motor_hw_get_speed(uint8_t id);		
int motor_hw_get_angle(uint8_t id);
float motor_hw_get_circly(uint8_t id);
int32_t motor_hw_get_encords(uint8_t id);
int32_t motor_hw_get_position(uint8_t id);
int32_t motor_hw_get_version(uint8_t id);
bool is_motor_base(uint8_t id);
void motor_hw_set_pwm_isr(uint8_t id,int pwm);
void motor_hw_stop_isr(uint8_t id,uint8_t mode);
void motor_close_all(void);
void free_dev_motor(uint8_t port);

/* 百分比与速度映射函数 */
float motor_percent_to_speed(uint8_t id, float percent);
float motor_speed_to_percent(uint8_t id, float speed);
float motor_hw_get_dt(uint8_t id);
 
void reset_motor_hw_angle(uint8_t id);

void motor_delay_exit(uint32_t tick);
 

void InitMotorBSP(void);
bool set_motor_targetPos(char *velocity,int uSpeed,uint8_t port,double degress,uint32_t enctordInfo);
const MOTOR_HW_INFO *getMotorHWinfo(uint8_t type);
void runTimer_motor(DEV_MOTOR *motor,uint8_t *data);
#endif
