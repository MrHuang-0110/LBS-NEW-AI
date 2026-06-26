#include "_motor.h"
#include "motor.h"
#include "gray.h"
#include "portagree.h"
#include "uart.h"
#include "mem.h"
#include "stdarg.h"
#include "PikaVM.h"
 
extern void USB_printf(const char *format, ...);
 
void _motor_set_double_motor_kc(PikaObj *self, pika_float kf)
{ 
   ;
}
void _motor_set_accel_limt(PikaObj *self, int port, int limit)
{ 
    DEV_MOTOR *_motor_ = get_motor_base(port);
    if(_motor_ == NULL) return; 
	  const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(_motor_->base.type);
	  if(motor_hw_info == NULL)return;

		//_motor_->control.accel_limit = limit;
}
void _motor_pid_pos_ifo(PikaObj *self, int port, pika_float kp, pika_float ki, pika_float kd)
{ 
    DEV_MOTOR *_motor_ = get_motor_base(port);
    if(_motor_ == NULL) return; 
	  const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(_motor_->base.type);
	  if(motor_hw_info == NULL)return;
	
    float max_speed = motor_hw_info->max_speed;
   
	   Pos_Init(&_motor_->control.control_position,kp,ki,50,100);
}

void _motor_pid_speed_info(PikaObj *self, int port, pika_float kp, pika_float ki, pika_float kd)
{ 
    DEV_MOTOR *_motor_ = get_motor_base(port);
    if(_motor_ == NULL) return; 
	  const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(_motor_->base.type);
	  if(motor_hw_info == NULL)return;
	
    float max_speed = motor_hw_info->max_speed;	

	  PI_Init(&_motor_->control.control_V50_Speed, kp,ki, 50.0f, 100.0f);				 
}

void _motor_pid_yaw_info(PikaObj *self, pika_float type, pika_float kp, pika_float ki, pika_float kd)
{ 
 _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();

	if(type == DEV_ID_BIG_MOTOR)
	{
 
		 PI_Init(&doubleMotor->yaw_pid,kp,ki,BIG_MOTOR_MAX_SPEED,BIG_MOTOR_MAX_SPEED * 0.5f);
	}
	else if(type == DEV_ID_SMALL_Motor)
	{
		 PI_Init(&doubleMotor->yaw_pid,kp,ki,SMALL_MOTOR_MAX_SPEED,SMALL_MOTOR_MAX_SPEED * 0.5f);
	}
}

void _motor_pid_angle_info(PikaObj *self, pika_float kp, pika_float ki, pika_float kd)

{ 
	   ;
}
int _motor_absolute_position(PikaObj *self, int port)
{   
    return motor_hw_get_position(port); 
}

int _motor_get_duty_cycle(PikaObj *self, int port)
{ 
     return motor_hw_get_speed(port);
}

int _motor_relative_angle(PikaObj *self, int port)
{ 
    return motor_hw_get_angle(port);
}

void _motor_reset_relative_position(PikaObj *self, int port)
{   
	  const uint32_t timeout_ms = 2000;
	  const uint32_t retry_ms = 100;
	  uint32_t start_tick = xTaskGetTickCount();

	  do {
		  reset_motor_hw_angle(port);
		  motor_delay_exit(retry_ms);
		  if(abs(motor_hw_get_angle(port)) <= 1) {
			  return;
		  }
	  } while((xTaskGetTickCount() - start_tick) < timeout_ms &&
	          VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT);
}

void _motor_run(PikaObj *self, int port, char* dir)
{ 
		DEV_MOTOR *_motor_ = get_motor_base(port);
    if(_motor_ == NULL) return; 	
	   const MOTOR_HW_INFO *motor_hw = getMotorHWinfo(_motor_->base.type);
    int tSpeed = _motor_->motor_user_info.uSpeed;
		if(strcmp(dir, "stop") == 0)
    { 
			_motor_->motor_user_info.temap_stop = _motor_->motor_user_info.stop_mode;
			_motor_->control.current_mode = MOTOR_STOP;     
        return;	    
    }
	 
    if(strcmp(dir, "retreat") == 0)
    { 
        tSpeed *= (-1);
    }		
		_motor_->motor_user_info.temap_uSpeed = tSpeed;
		_motor_->control.current_mode = MOTOR_SPEED;
}

void _motor_run_for_degrees(PikaObj *self, int port, char* dir, pika_float degrees, char* velocity)
{    
		DEV_MOTOR *_motor_ = get_motor_base(port);
    if(_motor_ == NULL) return; 	
	
 	 const MOTOR_HW_INFO *motor_hw_info = getMotorHWinfo(_motor_->base.type);
	 if(motor_hw_info == NULL)return;   
	
    _motor_->motor_user_info.temap_uSpeed = _motor_->motor_user_info.uSpeed;
	
//	  PID_Reset(&_motor_->control.control_speed);
//	  PID_Reset(&_motor_->control.control_pos);

		if(strcmp(dir, "stop") == 0)
    { 
			_motor_->motor_user_info.temap_stop = _motor_->motor_user_info.stop_mode;
			_motor_->control.current_mode = MOTOR_STOP;       
        return;	    
    }
	 
    if(strcmp(dir, "retreat") == 0)
    { 
        _motor_->motor_user_info.temap_uSpeed *= (-1);
    }				
		
		if(degrees < 0)
		{ 
		   _motor_->motor_user_info.temap_uSpeed *= (-1);
		}
 
		if(strcmp(velocity,"seconds") == 0)
		{ 
			_motor_->control.current_mode = MOTOR_SPEED;
      float ms_float = fabs(degrees) * 1000.0f;
      motor_delay_exit((uint32_t)ms_float);	
      _motor_->control.current_mode = MOTOR_STOP_BREAK;    			
		   return;
		}
		else
		{ 
		   bool state1 = set_motor_targetPos(velocity,
																				 _motor_->motor_user_info.temap_uSpeed,
																				 port,
																				 degrees,
																				 motor_hw_info->enctordinfo
																				);
 
			 if(state1!=true)return;
		}
 
 
		_motor_->control.current_mode = MOTOR_SPEED;

		bool motor_arrived = false; 
		while(!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
		{ 	 
			if(_motor_->motor_user_info.temap_uSpeed > 0)
			{ 
				motor_arrived = _motor_->motor_run_info.current_pos >= _motor_->control.target_position;
			}
		  else if(_motor_->motor_user_info.temap_uSpeed < 0)
			{ 
				motor_arrived = _motor_->motor_run_info.current_pos <= _motor_->control.target_position;
			} 
			motor_delay_exit(1);	
		}
    _motor_->control.current_mode = MOTOR_STOP_BREAK;    
}

void _motor_run_power(PikaObj *self, int port, pika_float duty, pika_float mode)

{    
    DEV_MOTOR *_motor_ = get_motor_base(port); 
    if(_motor_ == NULL) return;
	  
	  if(mode == 0)
		{ 
			_motor_->motor_user_info.pwm = duty;
			_motor_->control.current_mode = MOTOR_PWM;				
		}
		else if(mode == 1)
		{ 
		  _motor_->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(port,duty);
			_motor_->control.current_mode = MOTOR_SPEED;
		}
}

void _motor_stop(PikaObj *self, int port)
{ 
	  
    DEV_MOTOR *_motor_ = get_motor_base(port); 
    if(_motor_ == NULL) return;
	  
	  _motor_->motor_user_info.temap_stop = _motor_->motor_user_info.stop_mode;
 		_motor_->control.current_mode = MOTOR_STOP;
}

void _motor_stop_module(PikaObj *self, int port, int module)
{ 
	 
    DEV_MOTOR *_motor_ = get_motor_base(port); 
    if(_motor_ == NULL) return;
    
	  _motor_->motor_user_info.stop_mode = module;
}

void _motor_set_duty(PikaObj *self, int port, pika_float duty)
{   
 
    DEV_MOTOR *_motor_ = get_motor_base(port); 
    if(_motor_ == NULL) return;
	
	  _motor_->motor_user_info.uSpeed = (int)motor_percent_to_speed(port,duty);
	 
}

void _motor_mov(PikaObj *self, char* dir)
{ 	 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
  if(strcmp(dir, "stop") == 0)
  { 
			master->motor_user_info.temap_stop = doubleMotor->doubleMotor_StopModule;
			slave->motor_user_info.temap_stop = doubleMotor->doubleMotor_StopModule;
			master->control.current_mode = MOTOR_STOP;
			slave->control.current_mode = MOTOR_STOP;
        return;
  }
   
  master->motor_user_info.temap_uSpeed = doubleMotor->uSpeed[0];
  slave->motor_user_info.temap_uSpeed = doubleMotor->uSpeed[1];
	if(strcmp(dir, "advance") == 0)
	{ 
		 master->control.current_mode = MOTOR_SPEED;
		 slave->control.current_mode = MOTOR_SPEED;
		 return;
	}
  else if(strcmp(dir, "retreat") == 0)
  { 
      master->motor_user_info.temap_uSpeed *= (-1.0f);
      slave->motor_user_info.temap_uSpeed *= (-1.0f);   
		  master->control.current_mode = MOTOR_SPEED;
		  slave->control.current_mode = MOTOR_SPEED;
			return;			
  }
  else if(strcmp(dir, "right") == 0)
  { 
      slave->motor_user_info.temap_uSpeed *= (-1.0f);
		  master->control.current_mode = MOTOR_SPEED;
		  slave->control.current_mode = MOTOR_SPEED;
			return;
  }
  else if(strcmp(dir, "left") == 0)
  { 
       master->motor_user_info.temap_uSpeed *= (-1.0f);
			 master->control.current_mode = MOTOR_SPEED;
			 slave->control.current_mode = MOTOR_SPEED;
			  return;
  }
}

void _motor_mov_for_degrees(PikaObj *self, char* dir, pika_float degrees, char* velocity)
{
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
	
 	 const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
	 const MOTOR_HW_INFO *salve_hw_info = getMotorHWinfo(slave->base.type);
	 if(master_hw_info == NULL || salve_hw_info == NULL)return;  
  
   master->motor_user_info.temap_uSpeed = doubleMotor->uSpeed[0];
	 slave->motor_user_info.temap_uSpeed = doubleMotor->uSpeed[1];
 
	 if(strcmp(dir, "stop") == 0)
   { 
			master->control.current_mode = MOTOR_STOP;
			slave->control.current_mode = MOTOR_STOP;
        return;  
   }
	 
    if(strcmp(dir, "retreat") == 0)
    { 
        master->motor_user_info.temap_uSpeed *= (-1.0f);
        slave->motor_user_info.temap_uSpeed *= (-1.0f);    
    }
    else if(strcmp(dir, "right") == 0)
    { 
        slave->motor_user_info.temap_uSpeed *= (-1.0f);
    }
    else if(strcmp(dir, "left") == 0)
    { 
        master->motor_user_info.temap_uSpeed *= (-1.0f);
    }		
		
		if(degrees < 0)
		{ 
        master->motor_user_info.temap_uSpeed *= (-1.0f);
        slave->motor_user_info.temap_uSpeed *= (-1.0f);  
		}
 
		if(strcmp(velocity,"seconds") == 0)
		{ 
			master->control.current_mode = MOTOR_SPEED;
			slave->control.current_mode = MOTOR_SPEED;
      float ms_float = fabs(degrees) * 1000.0f;
      motor_delay_exit((uint32_t)ms_float);	
      master->control.current_mode = MOTOR_STOP_BREAK;    	
      slave->control.current_mode = MOTOR_STOP_BREAK;    			
		  return;
		}
		else
		{ 
		   bool state1 = set_motor_targetPos(velocity,
																				 master->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[0],
																				 degrees,
																				 master_hw_info->enctordinfo
																				);
			 
		   bool state2 = set_motor_targetPos(velocity,
																				 slave->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[1],
																				 degrees,
																				 salve_hw_info->enctordinfo
																				);
			 
			 if(state1!=true || state2!=true)return;
		}
 

		master->control.current_mode = MOTOR_SPEED;
		slave->control.current_mode = MOTOR_SPEED;	
		
		bool motor_arrived = false;
		while (!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT)) {
    bool master_exceed = false;
    bool slave_exceed  = false;

    // 主电机：只有速度非零时才判断是否超出目标
    if (master->motor_user_info.temap_uSpeed > 0) {
        master_exceed = (master->motor_run_info.current_pos > master->control.target_position);
    } else if (master->motor_user_info.temap_uSpeed < 0) {
        master_exceed = (master->motor_run_info.current_pos < master->control.target_position);
    } // 速度为0时 master_exceed 保持 false

    // 从电机：同理
    if (slave->motor_user_info.temap_uSpeed > 0) {
        slave_exceed = (slave->motor_run_info.current_pos > slave->control.target_position);
    } else if (slave->motor_user_info.temap_uSpeed < 0) {
        slave_exceed = (slave->motor_run_info.current_pos < slave->control.target_position);
    }
 
    motor_arrived = master_exceed && slave_exceed;

    motor_delay_exit(1);
}
    master->control.current_mode = MOTOR_STOP_BREAK; 
    slave->control.current_mode = MOTOR_STOP_BREAK;      	
}

void _motor_mov_for_power_degrees(PikaObj *self, pika_float duty1, pika_float duty2, pika_float degrees, char* velocity)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;

 	 const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
	 const MOTOR_HW_INFO *salve_hw_info = getMotorHWinfo(slave->base.type);
	 if(master_hw_info == NULL || salve_hw_info == NULL)return;  
	
   master->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[0],duty1);
	 slave->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[1],duty2);
	
		if(degrees < 0)
		{ 
        master->motor_user_info.temap_uSpeed *= (-1.0f);
        slave->motor_user_info.temap_uSpeed *= (-1.0f);  
		}
 
		if(strcmp(velocity,"seconds") == 0)
		{ 
			master->control.current_mode = MOTOR_SPEED;
			slave->control.current_mode = MOTOR_SPEED;
      float ms_float = fabs(degrees) * 1000.0f;
      motor_delay_exit((uint32_t)ms_float);	
      master->control.current_mode = MOTOR_STOP_BREAK;    	
      slave->control.current_mode = MOTOR_STOP_BREAK;    			
		  return;
		}
		else
		{ 
		   bool state1 = set_motor_targetPos(velocity,
																				 master->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[0],
																				 degrees,
																				 master_hw_info->enctordinfo
																				);
			 
		   bool state2 = set_motor_targetPos(velocity,
																				 slave->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[1],
																				 degrees,
																				 salve_hw_info->enctordinfo
																				);
			 
			 if(state1!=true || state2!=true)return;
		}
    
	 
		master->control.current_mode = MOTOR_SPEED;
		slave->control.current_mode = MOTOR_SPEED;	
		
		bool motor_arrived = false;
		while (!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT)) {
    bool master_exceed = false;
    bool slave_exceed  = false;

    // 主电机：只有速度非零时才判断是否超出目标
    if (master->motor_user_info.temap_uSpeed > 0) {
        master_exceed = (master->motor_run_info.current_pos > master->control.target_position);
    } else if (master->motor_user_info.temap_uSpeed < 0) {
        master_exceed = (master->motor_run_info.current_pos < master->control.target_position);
    } // 速度为0时 master_exceed 保持 false

    // 从电机：同理
    if (slave->motor_user_info.temap_uSpeed > 0) {
        slave_exceed = (slave->motor_run_info.current_pos > slave->control.target_position);
    } else if (slave->motor_user_info.temap_uSpeed < 0) {
        slave_exceed = (slave->motor_run_info.current_pos < slave->control.target_position);
    }

    // 只要任意一个运动的电机超出其目标方向，即认为到达
    motor_arrived = master_exceed || slave_exceed;

    motor_delay_exit(1);
}
    master->control.current_mode = MOTOR_STOP_BREAK; 
    slave->control.current_mode = MOTOR_STOP_BREAK;      		
}

void _motor_mov_power(PikaObj *self, pika_float duty1, pika_float duty2, pika_float mode)

{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
  
	  if(mode == 0)
		{ 
			master->motor_user_info.pwm = duty1;
			slave->motor_user_info.pwm = duty2;
			master->control.current_mode = MOTOR_PWM;			
      slave->control.current_mode = MOTOR_PWM;					
		}
		else if(mode == 1)
		{ 
		  master->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[0],duty1);
			slave->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[1],duty2);
			master->control.current_mode = MOTOR_SPEED;
			slave->control.current_mode = MOTOR_SPEED;
		}
}

void _motor_mov_set_duty(PikaObj *self, pika_float duty)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
	
	doubleMotor->uSpeed[0]= (int)motor_percent_to_speed(doubleMotor->id[0],duty);
	doubleMotor->uSpeed[1]= (int)motor_percent_to_speed(doubleMotor->id[1],duty);
}

void _motor_mov_set_power_duty(PikaObj *self, pika_float duty1, pika_float duty2)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
	
	doubleMotor->uSpeed[0]= (int)motor_percent_to_speed(doubleMotor->id[0],duty1);
	doubleMotor->uSpeed[1]= (int)motor_percent_to_speed(doubleMotor->id[1],duty2);
}

void _motor_mov_set_stop_module(PikaObj *self, int module)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
	
	doubleMotor->doubleMotor_StopModule = module;
}

void _motor_mov_stop(PikaObj *self)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;

  master->motor_user_info.temap_stop = 	doubleMotor->doubleMotor_StopModule;
	slave->motor_user_info.temap_stop = 	doubleMotor->doubleMotor_StopModule;
	
	master->control.current_mode = MOTOR_STOP;
	slave->control.current_mode = MOTOR_STOP;
}
 
void _motor_pair(PikaObj *self, int port1, int port2)
{ 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();

  DEV_MOTOR *master = get_motor_base(port1); 
  DEV_MOTOR *slave = get_motor_base(port2); 
	if(master == NULL || slave == NULL)return;
	
	doubleMotor->id[0] = port1,doubleMotor->id[1] = port2;
	doubleMotor->doubleMotor_StopModule = MOTOR_STOP_SILD;
	doubleMotor->uSpeed[0] = (int)motor_percent_to_speed(doubleMotor->id[0],50);
	doubleMotor->uSpeed[1] = (int)motor_percent_to_speed(doubleMotor->id[1],50);
	
	if(master->base.type == DEV_ID_BIG_MOTOR && slave->base.type == DEV_ID_BIG_MOTOR)
	{
	   PI_Init(&doubleMotor->yaw_pid,0,0,BIG_MOTOR_MAX_SPEED,BIG_MOTOR_MAX_SPEED * 0.5f);
	}
	else if(master->base.type == DEV_ID_SMALL_Motor && slave->base.type == DEV_ID_SMALL_Motor)
	{
		 PI_Init(&doubleMotor->yaw_pid,0,0,SMALL_MOTOR_MAX_SPEED,SMALL_MOTOR_MAX_SPEED * 0.5f);
	}
		
	doubleMotor->is_motor_handle = true;
}

void _motor_mov_yaw_line(PikaObj *self, char* dir, pika_float degrees, char* velocity)
{ 
  #define MAX_YAW_LINE_POWER_DUTY 80
	#define YAW_KP 5.0f
	#define YAW_KI 0.0f
 
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;

  TickType_t xCurrentTick;
  float dt;
	static TickType_t xLastTick;
  
	float current_s_tick = 0;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;

 	const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
	const MOTOR_HW_INFO *salve_hw_info = getMotorHWinfo(slave->base.type);
	if(master_hw_info == NULL || salve_hw_info == NULL)return;  
	 
	if(master->base.type == DEV_ID_BIG_MOTOR && slave->base.type == DEV_ID_BIG_MOTOR)
		 PI_Init(&doubleMotor->yaw_pid,YAW_KP,YAW_KI,BIG_MOTOR_MAX_SPEED,BIG_MOTOR_MAX_SPEED * 0.5f);
	else if(master->base.type == DEV_ID_SMALL_Motor && slave->base.type == DEV_ID_SMALL_Motor)
		 PI_Init(&doubleMotor->yaw_pid,YAW_KP,YAW_KI,SMALL_MOTOR_MAX_SPEED,SMALL_MOTOR_MAX_SPEED * 0.5f);
	
	 int base_power =motor_percent_to_speed(doubleMotor->id[0],MAX_YAW_LINE_POWER_DUTY);

	 if(strcmp(dir, "stop") == 0)
   { 
			master->control.current_mode = MOTOR_STOP;
			slave->control.current_mode = MOTOR_STOP;
      return;  
   } 
	 
   if(strcmp(dir, "retreat") == 0)
       base_power*=(-1);   
	 if(degrees < 0)
        base_power*=(-1);  
		
 
		resetyaw(); 
		pika_GIL_EXIT();
		vTaskDelay(10);
		pika_GIL_ENTER();	

	 master->motor_user_info.temap_uSpeed = base_power;
   slave->motor_user_info.temap_uSpeed = base_power;	
	 
		if(strcmp(velocity,"seconds") == 0)
		{ 
			current_s_tick = 0;
			TickType_t start_ticks = xTaskGetTickCount();
			while(current_s_tick < fabs(degrees))
			{ 
        if (VMSignal_getCtrl() == VM_SIGNAL_CTRL_EXIT) {
            break;
        }
			  xCurrentTick = xTaskGetTickCount();
				current_s_tick = (xCurrentTick - start_ticks) * portTICK_PERIOD_MS / 1000.0f;
				if(xLastTick == 0) {
            dt = 0.01f; 
        } else {
            dt = (xCurrentTick - xLastTick) * portTICK_PERIOD_MS / 1000.0f;
            if(dt <= 0) dt = 0.01f; 
        }
        xLastTick= xCurrentTick;		 
        
				float adjustment	= PI_Compute(&doubleMotor->yaw_pid,0,get_yaw(),dt);	

				if(master->base.type == DEV_ID_BIG_MOTOR && slave->base.type == DEV_ID_BIG_MOTOR)
				{ 
					if(adjustment > BIG_MOTOR_MAX_SPEED)adjustment = BIG_MOTOR_MAX_SPEED;
					if(adjustment < -BIG_MOTOR_MAX_SPEED)adjustment = -BIG_MOTOR_MAX_SPEED;						   
				}
				else if(master->base.type == DEV_ID_SMALL_Motor && slave->base.type == DEV_ID_SMALL_Motor)
				{ 
					if(adjustment > SMALL_MOTOR_MAX_SPEED)adjustment = SMALL_MOTOR_MAX_SPEED;
					if(adjustment < -SMALL_MOTOR_MAX_SPEED)adjustment = -SMALL_MOTOR_MAX_SPEED;						   
				}
				
				if(base_power > 0)
				{
						master->motor_user_info.temap_uSpeed = (int)base_power + (int)adjustment;
						slave->motor_user_info.temap_uSpeed = (int)base_power - (int)adjustment;					
				}
        else if(base_power < 0)
				{ 	
						master->motor_user_info.temap_uSpeed = (int)base_power - (int)adjustment * (-1);
						slave->motor_user_info.temap_uSpeed = (int)base_power + (int)adjustment * (-1);							
				}
				master->control.current_mode = MOTOR_SPEED;			
				slave->control.current_mode = MOTOR_SPEED;	

			   motor_delay_exit(1);
			}
 				master->control.current_mode = MOTOR_STOP_BREAK;			
				slave->control.current_mode = MOTOR_STOP_BREAK;					 
			  return;
		}
		else
		{ 
		   bool state1 = set_motor_targetPos(velocity,
																				 master->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[0],
																				 degrees,
																				 master_hw_info->enctordinfo
																				);
			 
		   bool state2 = set_motor_targetPos(velocity,
																				 slave->motor_user_info.temap_uSpeed,
																				 doubleMotor->id[1],
																				 degrees,
																				 salve_hw_info->enctordinfo
																				);
			 
			 if(state1!=true || state2!=true)return;
		}
 
		master->control.current_mode = MOTOR_SPEED;
		slave->control.current_mode = MOTOR_SPEED;	
		
		bool motor_arrived = false;
		while (!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT)) {
    bool master_exceed = false;
    bool slave_exceed  = false;
 	
 		xCurrentTick = xTaskGetTickCount();
    dt = (xCurrentTick - xLastTick) * portTICK_PERIOD_MS / 1000.0f;
    xLastTick= xCurrentTick;			
     
		float adjustment	= PI_Compute(&doubleMotor->yaw_pid,0,get_yaw(),dt);
				
		if(master->base.type == DEV_ID_BIG_MOTOR && slave->base.type == DEV_ID_BIG_MOTOR)
		{ 
			if(adjustment > BIG_MOTOR_MAX_SPEED)adjustment = BIG_MOTOR_MAX_SPEED;
			if(adjustment < -BIG_MOTOR_MAX_SPEED)adjustment = -BIG_MOTOR_MAX_SPEED;						   
		}
		else if(master->base.type == DEV_ID_SMALL_Motor && slave->base.type == DEV_ID_SMALL_Motor)
		{ 
			if(adjustment > SMALL_MOTOR_MAX_SPEED)adjustment = SMALL_MOTOR_MAX_SPEED;
			if(adjustment < -SMALL_MOTOR_MAX_SPEED)adjustment = -SMALL_MOTOR_MAX_SPEED;						   
		}
				
		if(base_power > 0)
		{			
					master->motor_user_info.temap_uSpeed = (int)base_power + (int)adjustment;
				  slave->motor_user_info.temap_uSpeed =  (int)base_power - (int)adjustment;			  
		}
    else if(base_power < 0)
		{ 				
					master->motor_user_info.temap_uSpeed = (int)base_power - (int)adjustment * -1;
				  slave->motor_user_info.temap_uSpeed =  (int)base_power + (int)adjustment * -1;					  
		}

    if (base_power > 0) {
        master_exceed = (master->motor_run_info.current_pos > master->control.target_position);
    }else if (base_power < 0) {
        master_exceed = (master->motor_run_info.current_pos < master->control.target_position);
    }
		
    if (base_power > 0) {
        slave_exceed = (slave->motor_run_info.current_pos > slave->control.target_position);
    }else if (base_power < 0) {
        slave_exceed = (slave->motor_run_info.current_pos < slave->control.target_position);
    }
		motor_arrived = master_exceed && slave_exceed;
    motor_delay_exit(1);
	}
    master->control.current_mode = MOTOR_STOP_BREAK; 
    slave->control.current_mode = MOTOR_STOP_BREAK;   
}

void _motor_mov_yaw_angle(PikaObj *self, pika_float angle)
{
    _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle) return;

    DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]);
    DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]);
    if(master == NULL || slave == NULL) return;

    DEV_MEM *mem = getMemHandle();
    static float last_raw_yaw = 0.0f;
    static float yaw_offset = 0.0f;
    float initial_yaw;

    for(uint8_t i = 0; i < 10; i++) {
        mem->is_refresh = false;
        while(!mem->is_refresh) {
            pika_GIL_EXIT();
            vTaskDelay(1);
            pika_GIL_ENTER();
        }
        initial_yaw = get_continuous_yaw(mem, &last_raw_yaw, &yaw_offset);
    }

    float total_rotated = 0;
    float last_continuous_yaw = initial_yaw;
    float remaining_angle = fabsf((float)angle);
    float target_continuous_yaw = initial_yaw + angle;

    int direction = (angle > 0) ? 1 : -1;

    // 速度控制参数
    #define MAX_SPEED_DUTY  100.0f        // 最大速度
    #define MIN_SPEED_DUTY  1.0f        // 最小速度
    const float DECEL_START_ANGLE = 180.0f; // 开始减速的角度
    const float STUCK_THRESHOLD = 5;   // 卡死检测阈值（度）
    const int STUCK_COUNT_MAX = 10;       // 连续卡死检测次数
    const float STUCK_BOOST_FACTOR = 2.5f; // 卡死时速度提升倍数
    
		float MAX_SPEED = motor_percent_to_speed(doubleMotor->id[0],MAX_SPEED_DUTY);
		float MIN_SPEED = motor_percent_to_speed(doubleMotor->id[0],MIN_SPEED_DUTY);
		
    // 防卡死变量
    static float prev_remaining_angle = 0.0f;
    static int stuck_counter = 0;
    static bool in_stuck_boost = false;
    static int stuck_boost_counter = 0;

    // 初始化防卡死变量
    prev_remaining_angle = remaining_angle;
    stuck_counter = 0;
    in_stuck_boost = false;
    stuck_boost_counter = 0;

    // **主循环**
    while(VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT &&
          (!is_motor_base(doubleMotor->id[0]) && !is_motor_base(doubleMotor->id[1]))) {

        float current_continuous_yaw = get_continuous_yaw(mem, &last_raw_yaw, &yaw_offset);
        float delta_angle = calculate_angle_difference(last_continuous_yaw, current_continuous_yaw);

        total_rotated += fabsf(delta_angle);
        last_continuous_yaw = current_continuous_yaw;
        remaining_angle = fabsf((float)angle) - total_rotated;

        // 检查是否到达目标
        if(remaining_angle < 1.0f) {
            // 到达目标：退出循环
            break;
        }

        // 计算基础速度（线性减速）
        float base_speed = MAX_SPEED;
        if (remaining_angle < DECEL_START_ANGLE) {
            // 线性减速：从MAX_SPEED降到MIN_SPEED
            float ratio = remaining_angle / DECEL_START_ANGLE;
            base_speed = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * ratio;
            // 限制在最小和最大速度之间
            if (base_speed < MIN_SPEED) base_speed = MIN_SPEED;
            if (base_speed > MAX_SPEED) base_speed = MAX_SPEED;
        }

        // 防卡死检测
        float angle_change = fabsf(prev_remaining_angle - remaining_angle);
        if (angle_change < STUCK_THRESHOLD) {
            stuck_counter++;
            if (stuck_counter >= STUCK_COUNT_MAX && !in_stuck_boost) {
                // 检测到卡死，进入加速模式
                in_stuck_boost = true;
                stuck_boost_counter = 5; // 加速5个循环
            }
        } else {
            // 角度有变化，重置卡死计数器
            stuck_counter = 0;
        }

        // 应用卡死加速（如果激活）
        float final_speed = base_speed;
        if (in_stuck_boost) {
            final_speed = base_speed * STUCK_BOOST_FACTOR;
            stuck_boost_counter--;
            if (stuck_boost_counter <= 0) {
                in_stuck_boost = false;
                stuck_counter = 0; // 重置卡死计数器
            }
        }
				
        prev_remaining_angle = remaining_angle;

        master->motor_user_info.temap_uSpeed = final_speed * direction;
        slave->motor_user_info.temap_uSpeed = -final_speed * direction;
        master->control.current_mode = MOTOR_SPEED;
        slave->control.current_mode = MOTOR_SPEED;

        pika_GIL_EXIT();
        vTaskDelay(1);
        pika_GIL_ENTER();
    }

    // 停止电机
    master->control.current_mode = MOTOR_STOP_BREAK;
    slave->control.current_mode = MOTOR_STOP_BREAK;
}

void _motor_mov_find_line_init(PikaObj *self)
{ 
	  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle) return;
	
		DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
		DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
		if(master == NULL || slave == NULL)return;

		if(master->base.type == DEV_ID_BIG_MOTOR && slave->base.type == DEV_ID_BIG_MOTOR)
		 PI_Init(&doubleMotor->pid_find_line,0,0,BIG_MOTOR_MAX_SPEED,BIG_MOTOR_MAX_SPEED * 0.5f);
		else if(master->base.type == DEV_ID_SMALL_Motor && slave->base.type == DEV_ID_SMALL_Motor)
		 PI_Init(&doubleMotor->pid_find_line,0,0,SMALL_MOTOR_MAX_SPEED,SMALL_MOTOR_MAX_SPEED * 0.5f);
    
		doubleMotor->pid_find_line.pid_prev_time = 0;
}


void _motor_mov_find_line_run(PikaObj *self, pika_float left, pika_float right,
                              pika_float power, pika_float kp, pika_float kd)
{
    _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle) return;

    DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]);
    DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]);
    if(master == NULL || slave == NULL) return;

    const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
    if(master_hw_info == NULL) return;

    // 获取实际时间差用于微分项计算
    uint32_t now = xTaskGetTickCount();    // 替换为实际的系统毫秒函数
    static uint32_t last_time = 0;
    float dt = 0.005f;                  // 默认周期（首次调用）
    if(last_time != 0) {
        dt = (now - last_time) / 1000.0f; // 秒
        if(dt > 0.05f) dt = 0.05f;       // 限制最大间隔
        if(dt < 0.001f) dt = 0.001f;     // 防止除零
    }
    last_time = now;

    // 计算误差
    float error = (left - right)/10.0f;

    // 后退时误差取反，使转向方向与前进一致
    if(power < 0) error = -error;

    // PD 控制
    float proportional = kp * error;
    float derivative = kd * (error - doubleMotor->pid_find_line.last_error) / dt;
    float adjustment = proportional + derivative;

    // 存储上次误差
    doubleMotor->pid_find_line.last_error = error;

    // 限制调节量，避免出现一个轮子反转
    float max_adjust = 200;
 
    if(adjustment > max_adjust) adjustment = max_adjust;
    if(adjustment < -max_adjust) adjustment = -max_adjust;

    // 计算左右轮功率（保持两轮同向，差速转向）
    float left_power = power + adjustment;
    float right_power = power - adjustment;

    // 限幅到电机允许范围
    if(left_power > 100.0f) left_power = 100.0f;
    if(left_power < -100.0f) left_power = -100.0f;
    if(right_power > 100.0f) right_power = 100.0f;
    if(right_power < -100.0f) right_power = -100.0f;

    // 应用输出
    master->motor_user_info.pwm = left_power;
    slave->motor_user_info.pwm = right_power;
    master->control.current_mode = MOTOR_PWM;
    slave->control.current_mode = MOTOR_PWM;
    motor_delay_exit(10);
}
#if 0
void _motor_mov_find_line_power_run(PikaObj *self, pika_float left, pika_float right, pika_float power1, pika_float power2, pika_float kp, pika_float kd)
{ 
	
		_USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle) return;

    DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]);
    DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]);
    if(master == NULL || slave == NULL) return;

    float error = (left - right)/10.0f;  
  
    float proportional = kp * error;
    float derivative = kd * (error - doubleMotor->pid_find_line.last_error ) / 0.01f;
		
		doubleMotor->pid_find_line.last_error = error;
		
    float adjustment = proportional + derivative;
 
		if(adjustment > 100)adjustment = 100;
		if(adjustment < -100)adjustment = -100;						   
 

    float left_power = fabs(power1) + adjustment;
    float right_power = fabs(power2) - adjustment;
    if (left_power > 100.0f) left_power = 100.0f;
    if (left_power <-100.0f) left_power = -100.0f;
    if (right_power > 100.0f) right_power = 100.0f;
    if (right_power < -100.0f) right_power =-100.0f;
    
		if(power1<0){ 
		  left_power = -left_power;
		} 
		if(power2<0){
			right_power = -right_power;
		}
		
		if(doubleMotor->uSpeed[1]<0)right_power = -right_power;
    master->motor_user_info.pwm = left_power;
	  slave->motor_user_info.pwm = right_power; 		
    master->control.current_mode = MOTOR_PWM;
    slave->control.current_mode = MOTOR_PWM;	
		motor_delay_exit(10);
}
#else
void _motor_mov_find_line_power_run(PikaObj *self, pika_float left, pika_float right,
                                    pika_float power1, pika_float power2,
                                    pika_float kp, pika_float kd)
{
    _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle) return;

    DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]);
    DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]);
    if(master == NULL || slave == NULL) return;

    // 动态时间差
    uint32_t now = xTaskGetTickCount();
    static uint32_t last_time = 0;
    float dt = 0.01f;  // 默认周期
    if(last_time != 0) {
        dt = (now - last_time) / 1000.0f;
        if(dt > 0.05f) dt = 0.05f;
        if(dt < 0.001f) dt = 0.001f;
    }
    last_time = now;

    // 误差计算与后退修正
    float error = (left - right) / 10.0f;
    float avg_power = (power1 + power2) * 0.5f;
    if(avg_power < 0) error = -error;

    // PD 控制
    float proportional = kp * error;
    float derivative = kd * (error - doubleMotor->pid_find_line.last_error) / dt;
    float adjustment = proportional + derivative;
    doubleMotor->pid_find_line.last_error = error;

    // 调节量限幅（可调整）
    if(adjustment > 200.0f) adjustment = 200.0f;
    if(adjustment < -200.0f) adjustment = -200.0f;

    // 直接应用调节量
    float left_power = power1 + adjustment;
    float right_power = power2 - adjustment;

    // PWM 限幅
    if(left_power > 100.0f) left_power = 100.0f;
    if(left_power < -100.0f) left_power = -100.0f;
    if(right_power > 100.0f) right_power = 100.0f;
    if(right_power < -100.0f) right_power = -100.0f;

    master->motor_user_info.pwm = left_power;
    slave->motor_user_info.pwm = right_power;
    master->control.current_mode = MOTOR_PWM;
    slave->control.current_mode = MOTOR_PWM;

    motor_delay_exit(10);
}
#endif
