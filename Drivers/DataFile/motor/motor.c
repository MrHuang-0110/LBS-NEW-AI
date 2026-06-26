#include "motor.h"
#include "gray.h"
#include "uart.h"
#include "mem.h"
#include "portagree.h"
#include "PikaVM.h"
#include <math.h>
 

extern void USB_printfISR(const char *format, ...);
static DEV_MOTOR __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_motor[8];
static _USER_DOUBLE_MOTOR double_motor;

static TickType_t xLastTick[8] = {0};

typedef struct __attribute__((packed)) {
	  int speed,pos,angle,version;
}motor_packet_t;

static const MOTOR_HW_INFO  big_motor_hw_info = {
   .enctordinfo = BIG_MOTOR_ENCTORDINFO,
	 .v50Speed = BIG_MOTOR_MAX_SPEED,
	 .max_speed = BIG_MOTOR_MAX_SPEED,
	 .min_speed = BIG_MOTOR_MIN_SPEED
};

static const MOTOR_HW_INFO  small_motor_hw_info = {
   .enctordinfo = SMALL_MOTOR_ENCTORDINFO,
	 .v50Speed = SMALL_MOTOR_MAX_SPEED,
	 .max_speed = SMALL_MOTOR_MAX_SPEED,
	 .min_speed = SMALL_MOTOR_MIN_SPEED
};


const MOTOR_HW_INFO *getMotorHWinfo(uint8_t type)
{ 
   switch(type)
	 { 
	   case DEV_ID_BIG_MOTOR:return &big_motor_hw_info;
		 case DEV_ID_SMALL_Motor:return &small_motor_hw_info;
		 default:break;
	 }
	 return NULL;
}
 /* 位置环控制函数实现 */
static void motor_set_target_position(uint8_t id, int32_t position) {
    DEV_MOTOR *motor = get_motor_base(id);
    if (motor == NULL) return;
    motor->control.target_position = position;
    motor->control.position_reached = false;
}

static void motor_set_target_velocity(uint8_t id, int32_t velocity) {
    DEV_MOTOR *motor = get_motor_base(id);
    if (motor == NULL) return;
    motor->control.target_velocity = velocity;
}
/* 百分比与速度映射函数实现 - 0%表示停止，1%~100%表示从min_speed到max_speed */
float motor_percent_to_speed(uint8_t id, float percent) {
    DEV_MOTOR *motor = get_motor_base(id);
    if (motor == NULL) return 0.0f;
	
	  const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(motor->base.type);
	  if(motor_hw_info == NULL)return 0.0f;    
	
    float max_speed = motor_hw_info->max_speed;
    float min_speed = motor_hw_info->min_speed;
    
    // 参数验证
    if (max_speed <= min_speed) {
        // 如果设置不合理，则使用默认值
        max_speed = 150.0f;
        min_speed = 30.0f;
    }
    
    if (min_speed <= 0.0f) {
        min_speed = 30.0f; // 确保最小速度大于0
    }
    
    /* 限制百分比范围 */
    percent = (percent < -100.0f) ? -100.0f : (percent > 100.0f) ? 100.0f : percent;
    
    /* 映射逻辑：
     * 0% → 0 (停止)
     * 1%~100% → min_speed~max_speed
     * -1%~-100% → -min_speed~-max_speed
     */
    
    // 处理正百分比
    if (percent > 0.0f) {
        if (percent <= 1.0f) {
            // 0%~1%：缓启动，从0线性增加到min_speed
            // 这样在1%时刚好达到min_speed
            return min_speed * (percent / 1.0f);
        } else {
            // 1%~100%：线性映射到min_speed~max_speed
            // 归一化到0~1：将1%~100%映射到0~1
            float normalized = (percent - 1.0f) / 99.0f;
            return min_speed + (max_speed - min_speed) * normalized;
        }
    }
    // 处理负百分比
    else if (percent < 0.0f) {
        float abs_percent = -percent; // 取绝对值
        
        if (abs_percent <= 1.0f) {
            // -1%~0%：缓启动，从0线性增加到-min_speed
            return -min_speed * (abs_percent / 1.0f);
        } else {
            // -100%~-1%：线性映射到-max_speed~-min_speed
            float normalized = (abs_percent - 1.0f) / 99.0f;
            return -(min_speed + (max_speed - min_speed) * normalized);
        }
    }
    // percent == 0
    else {
        return 0.0f;
    }
}

float motor_speed_to_percent(uint8_t id, float speed) {
    DEV_MOTOR *motor = get_motor_base(id);
    if (motor == NULL) return 0.0f;
    
	  const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(motor->base.type);
	  if(motor_hw_info == NULL)return 0.0f;
	
    float max_speed = motor_hw_info->max_speed;
    float min_speed = motor_hw_info->min_speed;
    
    // 参数验证
    if (max_speed <= min_speed) {
        max_speed = 150.0f;
        min_speed = 30.0f;
    }
    
    if (min_speed <= 0.0f) {
        min_speed = 30.0f;
    }
    
    /* 反向映射：将速度转换回百分比
     * 正速度：
     *   0~min_speed → 0%~1%
     *   min_speed~max_speed → 1%~100%
     * 负速度同理
     */
    
    // 处理正速度
    if (speed > 0.0f) {
        if (speed < min_speed) {
            // 0~min_speed之间：映射到0%~1%
            return (speed / min_speed) * 1.0f;
        } else if (speed <= max_speed) {
            // min_speed~max_speed之间：映射到1%~100%
            // 先归一化到0~1，再映射到1%~100%
            float normalized = (speed - min_speed) / (max_speed - min_speed);
            return 1.0f + normalized * 99.0f;
        } else {
            // 超过最大速度：返回100%
            return 100.0f;
        }
    }
    // 处理负速度
    else if (speed < 0.0f) {
        float abs_speed = -speed; // 取绝对值
        
        if (abs_speed < min_speed) {
            // 0~-min_speed之间：映射到-1%~0%
            return -(abs_speed / min_speed) * 1.0f;
        } else if (abs_speed <= max_speed) {
            // -min_speed~-max_speed之间：映射到-100%~-1%
            float normalized = (abs_speed - min_speed) / (max_speed - min_speed);
            return -(1.0f + normalized * 99.0f);
        } else {
            // 超过最大速度：返回-100%
            return -100.0f;
        }
    }
    // speed == 0
    else {
        return 0.0f;
    }
}

void motor_delay_exit(uint32_t tick)
{ 
	  TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t total_ms = (uint32_t)(tick);  
    if (total_ms == 0) return;
    
    uint32_t start_ticks = xTaskGetTickCount();
    uint32_t target_ticks = start_ticks + total_ms;
    
    pika_GIL_EXIT();
    
    while (xTaskGetTickCount() < target_ticks) {
        if (VMSignal_getCtrl() == VM_SIGNAL_CTRL_EXIT) {
            break;
        }
        uint32_t remaining_ticks = target_ticks - xTaskGetTickCount();
        uint32_t delay_ticks = (remaining_ticks > 1) ? 
                                1 : remaining_ticks;
        
        if (delay_ticks > 0) {
					  vTaskDelayUntil(&xLastWakeTime,delay_ticks);
        } else {
            break;
        }
    }   
    pika_GIL_ENTER();
}

void motor_hw_stop(uint8_t id,uint8_t mode)
{ 
	 int pwm1,pwm2;
	
	 switch(mode)
	 { 
	   case MOTOR_STOP_SILD:
			  pwm1 = 0;
		    pwm2 = 0;
		 break;
		 
		 case MOTOR_STOP_BREAK:
			  pwm1 = MAX_OUTPUT_PWM;
		    pwm2 = MAX_OUTPUT_PWM;
		 break;
		 
		 default:
			  pwm1 = MAX_OUTPUT_PWM;
		    pwm2 = MAX_OUTPUT_PWM;
     break;		 
	 }
   
	  uint8_t pwmStr[4];
	  memset(pwmStr,0,sizeof(pwmStr));

    pwmStr[0] = (uint8_t)(pwm1 >> 8);
    pwmStr[1] = (uint8_t)(pwm1 & 0xFF);
    pwmStr[2] = (uint8_t)(pwm2 >> 8);
    pwmStr[3] = (uint8_t)(pwm2 & 0xFF);
	   
		MultiUart_SendFrame((UartDevice_t )id,pwmStr,4,DEV_ID_BIG_MOTOR|DEV_ID_SMALL_Motor,0xED,10,250); 
} 
 

void motor_hw_set_pwm(uint8_t id,int pwm)
{ 
   int pwm1,pwm2,output;
	
	 if(pwm > MAX_PWM)
		   pwm = 100;
	 if(pwm < -MAX_PWM)
		   pwm = -100;
	 
	  output = abs(pwm);
	 
		if(pwm > 0)
		{
		   pwm1 = output;
		   pwm2 = 0;
		}
		else if(pwm < 0)
		{
		   pwm1 = 0;
		   pwm2 = output;
		}
		else
		{
		   pwm1 = 0;
		   pwm2 = 0;
		}

	  uint8_t pwmStr[4];
	  memset(pwmStr,0,sizeof(pwmStr));

    pwmStr[0] = (uint8_t)(pwm1 >> 8);
    pwmStr[1] = (uint8_t)(pwm1 & 0xFF);
    pwmStr[2] = (uint8_t)(pwm2 >> 8);
    pwmStr[3] = (uint8_t)(pwm2 & 0xFF);
	   
	MultiUart_SendFrame((UartDevice_t )id,pwmStr,4,DEV_ID_BIG_MOTOR|DEV_ID_SMALL_Motor,0xED,10,50);
}


int motor_hw_get_speed(uint8_t id)
{ 
   DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	 if(_motor_!=NULL)
		  return _motor_->motor_run_info.speed; 
	 
	 return 0;
}

int32_t motor_hw_get_position(uint8_t id)
{ 
  DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->motor_run_info.current_pos;
	
	return 0;
}

int motor_hw_get_angle(uint8_t id)
{
  DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->motor_run_info.angle;  
	
	return 0;
}
void reset_motor_hw_angle(uint8_t id)
{
	uint8_t data[1] = {0x01};
  MultiUart_SendFrame((UartDevice_t )id,data,1,DEV_ID_BIG_MOTOR|DEV_ID_SMALL_Motor,0xDD,10,250); 
}
float motor_hw_get_circly(uint8_t id)
{ 
   DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->motor_run_info.circle;  
	
	return 0;  
}
int32_t motor_hw_get_encords(uint8_t id)
{ 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->motor_run_info.current_encords;  
	
	return 0;   
}
int32_t motor_hw_get_version(uint8_t id)
{ 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->motor_run_info.version;  
	
	return 0;      
}

float motor_hw_get_dt(uint8_t id)
{ 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(id));
	if(_motor_!=NULL)
		 return _motor_->control_dt;  
	
	return 0;      
}

bool set_motor_targetPos(char *velocity,int uSpeed,uint8_t port,double degress,uint32_t enctordInfo)
{ 
		 if(strcmp(velocity,"circly") == 0)
		{ 
			 if(uSpeed > 0)
					motor_set_target_position(port,motor_hw_get_position(port) + fabs((float)degress) * enctordInfo);
			 else if(uSpeed < 0)
				  motor_set_target_position(port,motor_hw_get_position(port) - fabs((float)degress) * enctordInfo);
			 return true;
		}
	   else if(strcmp(velocity,"angle") == 0)
		{ 
			 if(uSpeed > 0)
				motor_set_target_position(port,motor_hw_get_position(port) + (fabs((float)degress)/360.0f)*enctordInfo);
			 else if(uSpeed < 0)
				 motor_set_target_position(port,motor_hw_get_position(port) - (fabs((float)degress)/360.0f)*enctordInfo);
			  return true;
		}   
		return false;
}

void motor_close_all(void)
{ 
   for(uint8_t i = 0;i<8;i++)
	 { 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(i));
	  if(_motor_!=NULL){		 
			 _motor_->motor_user_info.stop_mode = MOTOR_STOP_BREAK;
		   _motor_->control.current_mode = MOTOR_STOP_BREAK;			 
		}    
	 }
	 vTaskDelay(200);
   for(uint8_t i = 0;i<8;i++)
	 { 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(i));
	  if(_motor_!=NULL){		 
			 _motor_->motor_user_info.stop_mode = MOTOR_STOP_SILD;
		   _motor_->control.current_mode = MOTOR_STOP_SILD;			 
		}    
	 }
   vTaskDelay(100);
   for(uint8_t i = 0;i<8;i++)
	 { 
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(i));
	  if(_motor_!=NULL){		 
		   _motor_->control.current_mode = MOTOR_IDLE;			 
		}    
	 }	 
}
bool is_motor_base(uint8_t id)
{ 
	return (read_motor((SensorBase *)getDevBase(id))!=NULL?false:true);
}
DEV_MOTOR *get_motor_base(uint8_t id)
{ 
  return (read_motor((SensorBase *)getDevBase(id)));
}

DEV_MOTOR *read_motor(void *self)
{ 
	 if(self == NULL)
		   return NULL; 
	 DEV_MOTOR *mt = (DEV_MOTOR*)self;
	 return mt;
}

void refsh_motor(DEV_MOTOR *motor,uint8_t *data)
{ 
   TickType_t xCurrentTick;
   float dt;
   float output,velocity_setpoint,targetSpeed,currentSpeed,feedforward_power,pi_correction;
	 const MOTOR_HW_INFO *motor_hw = getMotorHWinfo(motor->base.type);
	 float error;
	
	 xCurrentTick = xTaskGetTickCount();
   dt = (xCurrentTick - xLastTick[motor->motor_run_info.dev_id]) * portTICK_PERIOD_MS / 1000.0f;
   xLastTick[motor->motor_run_info.dev_id] = xCurrentTick;
	 motor_packet_t pkt;
   memcpy(&pkt, data, sizeof(motor_packet_t)); 

	 motor->motor_run_info.current_pos = pkt.pos;
	 motor->motor_run_info.speed = pkt.speed;
	// motor->motor_run_info.angle = pkt.angle;
	 motor->motor_run_info.version = pkt.version;
	      
//	 sscanf((const char*)data,"%d/%d/%d/%d",&motor->motor_run_info.current_pos,&motor->motor_run_info.speed,&motor->motor_run_info.angle,&motor->motor_run_info.version);
	 motor->motor_run_info.circle = (float)((float)pkt.pos / (float)motor_hw->enctordinfo);	 
	 motor->motor_run_info.angle = (int)(motor->motor_run_info.circle*360.0f);
	 if(motor->motor_user_info.temap_uSpeed!=motor->motor_user_info.last_temap_uSpeed)
	 { 	 
		  PI_Reset(&motor->control.control_V50_Speed);
	    motor->motor_user_info.last_temap_uSpeed = motor->motor_user_info.temap_uSpeed;
	 } 
	 switch(motor->control.current_mode)
	 { 
		case MOTOR_IDLE:
		 ; 
		break;
		
		case MOTOR_STOP:
		  motor_hw_stop(motor->motor_run_info.dev_id,motor->motor_user_info.temap_stop);
		break;
		
		case MOTOR_STOP_BREAK:
		   motor_hw_stop(motor->motor_run_info.dev_id,MOTOR_STOP_BREAK);
		break;	 
		
		case MOTOR_STOP_SILD:
		   motor_hw_stop(motor->motor_run_info.dev_id,MOTOR_STOP_SILD);
		break;

		case MOTOR_SPEED:		
		        targetSpeed = (float)motor->motor_user_info.temap_uSpeed;
	          currentSpeed = (float)motor->motor_run_info.speed;
		        feedforward_power = (float)((targetSpeed / motor_hw->v50Speed) * 50.0f);
						pi_correction = PI_Compute(&motor->control.control_V50_Speed, targetSpeed, currentSpeed,dt);
		        output =  feedforward_power + pi_correction;
            motor->motor_user_info.pwm = (int)output;
            motor_hw_set_pwm(motor->motor_run_info.dev_id, motor->motor_user_info.pwm);	
				    motor->control_dt = dt;
   break;
 
   case MOTOR_SPEED_POS:
		         #if 0
						 error = motor->control.target_position - motor->motor_run_info.current_pos;
							if (fabs(error) > 100) {
							velocity_setpoint = (error > 0) ? abs(motor->motor_user_info.temap_uSpeed) : -abs(motor->motor_user_info.temap_uSpeed);
						 } else {
							velocity_setpoint = (motor->motor_user_info.temap_uSpeed / 100) * error;
						 }
						if (velocity_setpoint > abs(motor->motor_user_info.temap_uSpeed)) velocity_setpoint = abs(motor->motor_user_info.temap_uSpeed);
						if (velocity_setpoint < -abs(motor->motor_user_info.temap_uSpeed)) velocity_setpoint = -abs(motor->motor_user_info.temap_uSpeed);		
						feedforward_power = (velocity_setpoint /  motor_hw->v50Speed) * 50.0f;
						pi_correction = PI_Compute(&motor->control.control_V50_Speed, velocity_setpoint, currentSpeed, dt);
						output = feedforward_power + pi_correction;
						if (output > 100) output = 100;
						if (output < -100) output = -100;
						 
            motor->motor_user_info.pwm = (int)output;
            motor_hw_set_pwm_isr(motor->motor_run_info.dev_id, motor->motor_user_info.pwm);	
				    motor->control_dt = dt;		
           #endif	 
   break;
   case MOTOR_PWM:
		 motor_hw_set_pwm(motor->motor_run_info.dev_id,
											motor->motor_user_info.pwm);
	    motor->control_dt = dt;
   break;		 
	 }
}


void free_dev_motor(uint8_t port)
{
   DEV_MOTOR *motor = read_motor((SensorBase *)getDevBase(port));	
   if(motor!=NULL)
   {
	    memset((DEV_MOTOR *)motor,0,sizeof(DEV_MOTOR));
	 }		  
}
 
DEV_MOTOR *create_small_motor(uint8_t id)
{ 
	DEV_MOTOR *motor = &dev_motor[id];
     
	 
	 motor->base.type = DEV_ID_SMALL_Motor;
   motor->base.devId = id;
	
	 memset(motor->base.name,0,sizeof(motor->base.name));
	 strcpy(motor->base.name,"small_motor"); 
 
	 motor->motor_run_info.dev_id = id;
	 motor->control.current_mode = MOTOR_IDLE;
 
	 motor->motor_user_info.stop_mode = MOTOR_STOP_SILD;
   motor->motor_user_info.uSpeed =  (int)(SMALL_MOTOR_MAX_SPEED/2.0f);
 
   motor->motor_user_info.pwm = 0;

 
   motor->control.target_position = 0;
   motor->control.target_velocity = 0;
   motor->control.max_velocity = (int)SMALL_MOTOR_MAX_SPEED;
   motor->control.current_position = 0;
   motor->control.current_velocity = 0;
   motor->control.pwm_output = 0;
   motor->control.enable = true;
   motor->control.position_reached = false;
   motor->control.position_tolerance = 5;
 
						 
	 PI_Init(&motor->control.control_V50_Speed, 0.25f, 0.55f, 50.0f, 100.0f);				 
	 return motor;			
}

DEV_MOTOR *create_big_motor(uint8_t id)
{ 
 	 DEV_MOTOR *motor = &dev_motor[id];
     
	 
	 motor->base.type = DEV_ID_BIG_MOTOR;
   motor->base.devId = id;
	
	 memset(motor->base.name,0,sizeof(motor->base.name));
	 strcpy(motor->base.name,"big_motor"); 
 
	 motor->motor_run_info.dev_id = id;
	 motor->control.current_mode = MOTOR_IDLE;
 
	 motor->motor_user_info.stop_mode = MOTOR_STOP_SILD;
   motor->motor_user_info.uSpeed =  (int)(BIG_MOTOR_MAX_SPEED/2.0f);
 
   motor->motor_user_info.pwm = 0;

 
   motor->control.target_position = 0;
   motor->control.target_velocity = 0;
   motor->control.max_velocity = (int)BIG_MOTOR_MAX_SPEED;
   motor->control.current_position = 0;
   motor->control.current_velocity = 0;
   motor->control.pwm_output = 0;
   motor->control.enable = true;
   motor->control.position_reached = false;
   motor->control.position_tolerance = 5;
	 
  // Pos_Init(&motor->control.control_position, 0.0f,0.0f,50.0f,100.0f);
	 PI_Init(&motor->control.control_V50_Speed, 1.5f, 0.5f, 50.0f, 100.0f);						 
	 return motor;		
}


_USER_DOUBLE_MOTOR *getDoubleMotorHandle(void)
{
  return &double_motor;
}
 

void InitMotorBSP(void)
{ 
	;
}
 
 
