#include "_grayv2.h"
#include "grayv2.h"
#include "motor.h"
#include "portagree.h"
#include "uart.h"
#include "PikaVM.h"
#define CALIBRATE_MODE_START 0xD0
#define SET_LED_RGB          0xD1
#define SET_THRESHOLD   		 0xD2

 
pika_float _grayv2_if_ch_black(PikaObj *self, int poirt, int ch)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(poirt));
	  if(grayv2==NULL)return 0;
    
	  if(ch >= 7)return 0;
	
    return (pika_float)(grayv2->L_State[ch] == 1); 
}

pika_float _grayv2_read_ch(PikaObj *self, int port, int ch)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return 0;
	  
	  return grayv2->L_Value[ch];
}

pika_float _grayv2_if_all_ch_way_state(PikaObj *self, int port, int type)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return 0;

		return (pika_float)getWayType(port,type);
}

void _grayv2_set_rgb(PikaObj *self, int port, int color)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;
	  
		typedef struct __attribute__((packed)) {
				uint8_t  rgb; 
		}set_cfg_pake;
		set_cfg_pake paket;
		
		paket.rgb = color;
	 
		MultiUart_SendFrame((UartDevice_t)port,
										(uint8_t*)&paket,
										sizeof(paket),
								    DEV_ID_GRAY_V2,
										SET_LED_RGB,
										10,250);	
    motor_delay_exit(10);											
}
void _grayv2_set_ch_rgb(PikaObj *self, int port, int ch1, int ch2, int ch3, int ch4, int ch5, int ch6, int ch7)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;
	  
    ; 
}

void _grayv2_start_calibrate(PikaObj *self, int port)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;
	  
		typedef struct __attribute__((packed)) {
				uint8_t  start_calibrate; 
		}set_cfg_pake;
		set_cfg_pake paket;
		
		paket.start_calibrate = 1;

		MultiUart_SendFrame((UartDevice_t)port,
										(uint8_t*)&paket,
										sizeof(paket),
								    DEV_ID_GRAY_V2,
										CALIBRATE_MODE_START,
										10,250);
										
		pika_GIL_EXIT();	
    vTaskDelay(500);		
    pika_GIL_ENTER();
						
		while(!grayv2->is_calibrate && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
    { motor_delay_exit(10);	}			
    										
}

void _grayv2_set_threshold(PikaObj *self, int port, int ch, int value)
{ 
    DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;
	  
		typedef struct __attribute__((packed)) {
				uint16_t threshold;
				uint8_t  ch; 
		}set_cfg_pake;
		
		set_cfg_pake paket;
		paket.ch = ch;
		paket.threshold = value;
		
		MultiUart_SendFrame((UartDevice_t)port,
												(uint8_t*)&paket,
												sizeof(paket),
												DEV_ID_GRAY_V2,
												SET_THRESHOLD,
												10,250);
   motor_delay_exit(10);	  												
}

void _grayv2_power_find_way_type(PikaObj *self, int port, pika_float power, int type, int enctord)
{ 
	 DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	 if(grayv2==NULL)return;
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;

	const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
	const MOTOR_HW_INFO *salve_hw_info = getMotorHWinfo(slave->base.type);
	
	 
	 while(!getWayType(port,type) && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
	 {
     grayv2_find_line(grayv2,
											doubleMotor,
											master,
											slave,
											power);
		 
				motor_delay_exit(10);	  
	 }
 	 
	 if(enctord == 0)
	 { 
		 master->control.current_mode = MOTOR_STOP_BREAK; 
		 slave->control.current_mode = MOTOR_STOP_BREAK;    
     return;		 
	 }
	 
	master->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[0],power);
	slave->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[1],power);
		
  bool state1 = set_motor_targetPos("angle",
																		master->motor_user_info.temap_uSpeed,
																		 doubleMotor->id[0],
																		 (360.0f / master_hw_info->enctordinfo) * enctord,
																			master_hw_info->enctordinfo
																		);
			 
	 bool state2 = set_motor_targetPos("angle",
																		 slave->motor_user_info.temap_uSpeed,
																			doubleMotor->id[1],
																			(360.0f / salve_hw_info->enctordinfo) * enctord,
																			 salve_hw_info->enctordinfo
																		 );	 
  if(state1!=true || state2!=true){return;} 
	 bool motor_arrived = false;
	 while (!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
	 {
			bool master_exceed = false;
			bool slave_exceed  = false;
      if (master->motor_user_info.temap_uSpeed > 0) {
        master_exceed = (master->motor_run_info.current_pos > master->control.target_position);
      } else if (master->motor_user_info.temap_uSpeed < 0) {
        master_exceed = (master->motor_run_info.current_pos < master->control.target_position);
      } 
      if (slave->motor_user_info.temap_uSpeed > 0) {
        slave_exceed = (slave->motor_run_info.current_pos > slave->control.target_position);
      } else if (slave->motor_user_info.temap_uSpeed < 0) {
        slave_exceed = (slave->motor_run_info.current_pos < slave->control.target_position);
      } 
		 #if 0
     grayv2_find_line(grayv2,
											doubleMotor,
											master,
											slave, 
											power);
		 #endif
	
 		 master->control.current_mode = MOTOR_SPEED;
		 slave->control.current_mode = MOTOR_SPEED;
			
		 motor_arrived = master_exceed && slave_exceed;
		 motor_delay_exit(10);	  
	 }  
 
   master->control.current_mode = MOTOR_STOP_BREAK; 
   slave->control.current_mode = MOTOR_STOP_BREAK;     
}

void _grayv2_power_find_if_ch_state(PikaObj *self, int port, pika_float power1, pika_float power2, int ch)
{ 
 	 DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	 if(grayv2==NULL)return;
	  if(ch >= 7)return;
	
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
 
	 master->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[0],power1);
	 slave->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[1],power2);

	 while(!grayv2->L_State[ch] && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
	 {
		 master->control.current_mode = MOTOR_SPEED;
		 slave->control.current_mode = MOTOR_SPEED;		 
	   motor_delay_exit(10);	  
	 }
    master->control.current_mode = MOTOR_STOP_BREAK; 
    slave->control.current_mode = MOTOR_STOP_BREAK;  
}

void _grayv2_power_find_line(PikaObj *self, int port, pika_float power)
{ 
	 DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	 if(grayv2==NULL)return;
  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
  if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	if(master == NULL || slave == NULL)return;
	
     grayv2_find_line(grayv2,
											doubleMotor,
											master,
											slave,
											power);
	
    motor_delay_exit(10);	
}
void _grayv2_power_find_line_encord(PikaObj *self, int port, pika_float power, int encord)
{ 
	 DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	 if(grayv2==NULL)return;

	 _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
   if(!doubleMotor->is_motor_handle)return;
	
  DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
  DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	
	if(master == NULL || slave == NULL)return;
	
 	const MOTOR_HW_INFO *master_hw_info = getMotorHWinfo(master->base.type);
	const MOTOR_HW_INFO *salve_hw_info = getMotorHWinfo(slave->base.type);
 

	 master->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[0],power);
	 slave->motor_user_info.temap_uSpeed = (int)motor_percent_to_speed(doubleMotor->id[1],power);
	
  bool state1 = set_motor_targetPos("angle",
																		 master->motor_user_info.temap_uSpeed,
																		 doubleMotor->id[0],
																		 (360.0f / master_hw_info->enctordinfo) * encord,
																			master_hw_info->enctordinfo
																		);
			 
	 bool state2 = set_motor_targetPos("angle",
																		  slave->motor_user_info.temap_uSpeed,
																			doubleMotor->id[1],
																			(360.0f / salve_hw_info->enctordinfo) * encord,
																			 salve_hw_info->enctordinfo
																		 );	 
	 if(state1!=true || state2!=true)return;
	 
	 bool motor_arrived = false;
	 while (!motor_arrived && (VMSignal_getCtrl() != VM_SIGNAL_CTRL_EXIT))
	 {
			bool master_exceed = false;
			bool slave_exceed  = false;
      if (master->motor_user_info.temap_uSpeed > 0) {
        master_exceed = (master->motor_run_info.current_pos > master->control.target_position);
      } else if (master->motor_user_info.temap_uSpeed < 0) {
        master_exceed = (master->motor_run_info.current_pos < master->control.target_position);
      } 
      if (slave->motor_user_info.temap_uSpeed > 0) {
        slave_exceed = (slave->motor_run_info.current_pos > slave->control.target_position);
      } else if (slave->motor_user_info.temap_uSpeed < 0) {
        slave_exceed = (slave->motor_run_info.current_pos < slave->control.target_position);
      } 
     grayv2_find_line(grayv2,
											doubleMotor,
											master,
											slave,
											power);
		 motor_arrived = master_exceed && slave_exceed;
			 motor_delay_exit(10);	  
	 }  
	  master->control.current_mode = MOTOR_STOP_BREAK; 
    slave->control.current_mode = MOTOR_STOP_BREAK;  
}

void _grayv2_power_find_line_ms(PikaObj *self, int port, pika_float power, pika_float ms)
{
		DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;

	  _USER_DOUBLE_MOTOR *doubleMotor = getDoubleMotorHandle();
    if(!doubleMotor->is_motor_handle)return;
	
    DEV_MOTOR *master = get_motor_base(doubleMotor->id[0]); 
    DEV_MOTOR *slave = get_motor_base(doubleMotor->id[1]); 
	  if(master == NULL || slave == NULL)return;
  	
		TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t total_ms = (uint32_t)(ms);  
    if (total_ms == 0) return;
    
    uint32_t start_ticks = xTaskGetTickCount();
    uint32_t target_ticks = start_ticks + total_ms;

    while (xTaskGetTickCount() < target_ticks) {
			
        if (VMSignal_getCtrl() == VM_SIGNAL_CTRL_EXIT) {
            break;
        }
				
        uint32_t remaining_ticks = target_ticks - xTaskGetTickCount();
        uint32_t delay_ticks = (remaining_ticks > 10) ? 
                                10: remaining_ticks;

				grayv2_find_line(grayv2,
												 doubleMotor,
											   master,
											   slave,
											   power);
				
        if (delay_ticks > 0) {
					  pika_GIL_EXIT();
					  vTaskDelayUntil(&xLastWakeTime,delay_ticks);
					  pika_GIL_ENTER();
        } else {
            break;
        }
				
	
    }   
     
	
    master->control.current_mode = MOTOR_STOP_BREAK;    	
    slave->control.current_mode = MOTOR_STOP_BREAK;     
} 

void _grayv2_set_pid(PikaObj *self, int port, pika_float kp, pika_float kd)
{ 
 		DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;

    grayv2->kp = kp;
    grayv2->kd = kd;	
}
void _grayv2_set_find_line_ch(PikaObj *self, int port, int lindex, int mindex, int rindex)
{ 
 		DEV_GRAYV2 *grayv2 = read_gray_v2((SensorBase *)getDevBase(port));
	  if(grayv2==NULL)return;

    grayv2->lindex = lindex;
	  grayv2->mindex = mindex;
	  grayv2->rindex = rindex;
}
