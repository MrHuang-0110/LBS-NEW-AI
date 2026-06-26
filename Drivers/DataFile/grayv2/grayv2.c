#include "grayv2.h"
 
#include "portagree.h"

static DEV_GRAYV2 __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_gray_v2[8];
 
typedef struct __attribute__((packed)) {
	  uint32_t version;
	  uint32_t threshold[7];
    uint16_t values[7];  // 归一化值 0~999
    uint8_t  states[7];  // 黑白状态 0=白/1=黑
	  uint8_t  is_calibrate;
} gray_packet_t;

 

uint8_t getWayType(int port,WAY_TYPE type)
{ 
	DEV_GRAYV2 *gray = read_gray_v2((SensorBase *)getDevBase(port));
  if(gray == NULL)return 0;
	
	 
	uint8_t ch_state = (gray->L_State[0] << 0) |
										 (gray->L_State[1] << 1) |
										 (gray->L_State[2] << 2) |
										 (gray->L_State[3] << 3) |
                     (gray->L_State[4] << 4) |
                     (gray->L_State[5] << 5) |
                     (gray->L_State[6] << 6);
	uint8_t maske = 0;
   switch((uint8_t)type)
	 { 
	   case T_WAY:
			 maske = 1<<6 | 1<<0;
		 break;
		 case LEFT_WAY:
			 maske = 1<<0;
		 break;
		 case RIGHT_WAY:
			 maske = 1<<6;
		 break;
		 case MIDDLE_WAY:
			 maske = 1<<4 | 1<<3 | 1<<2;
		 break;
		 case ONE_WAY:
			 maske = 1<<0;
		 break;
		 case TWO_WAY:
			 maske = 1<<1;
		 break;
	   case TREE_WAY:
			 maske = 1<<2;
		 break;
	   case FOURE_WAY:
			 maske = 1<<3;
		 break;
	   case FIVE_WAY:
			 maske = 1<<4;
		 break;
	   case SIX_WAY:
			 maske = 1<<5;
		 break;
	   case SEVEN_WAY:
			 maske = 1<<6;
		 break;
	 }
	 return ((ch_state & maske)==maske);
}
#if 1
void grayv2_find_line(DEV_GRAYV2 *grayv2,_USER_DOUBLE_MOTOR *doubleMotor,DEV_MOTOR *master,DEV_MOTOR *slave,float power)
{ 
 
				uint32_t now = xTaskGetTickCount();    // 替换为实际的系统毫秒函数
				static uint32_t last_time = 0;
				float dt = 0.005f;                  // 默认周期（首次调用）
				if(last_time != 0) {
						dt = (now - last_time) / 1000.0f; // 秒
						if(dt > 0.05f) dt = 0.05f;       // 限制最大间隔
						if(dt < 0.001f) dt = 0.001f;     // 防止除零
				}
				last_time = now;
				
 				float left_error = grayv2->L_Value[grayv2->lindex] - grayv2->L_Value[grayv2->mindex];
		    float right_error = grayv2->L_Value[grayv2->rindex] - grayv2->L_Value[grayv2->mindex];
		
				float error = (left_error - right_error)/10.0f;  
			
				if(power < 0) error = -error;
				
				float proportional = grayv2->kp * error;
				float derivative = grayv2->kd * (error - doubleMotor->pid_find_line.last_error ) /dt;
				
			 
				doubleMotor->pid_find_line.last_error = error;
				
				float adjustment = proportional + derivative;
		 
				if(adjustment > 400.0f)adjustment = 400.0f;
				if(adjustment < -400.0f)adjustment = -400.0f;						   
		 
			 			
				float left_power = power + adjustment;
				float right_power = power - adjustment;
				
			//	USB_printf("kp:%.2f,kd:%.2f,adjustment:%.2f,power1:%.2f,power2:%.2f\r\n",grayv2->kp,grayv2->kd,adjustment,left_power,right_power);
				if (left_power > 100.0f) left_power = 100.0f;
				if (left_power <-100.0f) left_power = -100.0f;
				if (right_power > 100.0f) right_power = 100.0f;
				if (right_power < -100.0f) right_power =-100.0f;
				
				
				master->motor_user_info.pwm = left_power;
				slave->motor_user_info.pwm = right_power; 	
				master->control.current_mode = MOTOR_PWM;
				slave->control.current_mode = MOTOR_PWM;	
 
}
#endif

//void grayv2_find_line(DEV_GRAYV2 *grayv2, _USER_DOUBLE_MOTOR *doubleMotor,
//                      DEV_MOTOR *master, DEV_MOTOR *slave, float power)
//{
//    // 计算时间间隔 dt（秒）
//    uint32_t now = xTaskGetTickCount();
//    static uint32_t last_time = 0;
//    float dt = 0.005f;                  // 默认周期（首次调用）
//    if (last_time != 0) {
//        dt = (now - last_time) / 1000.0f;
//        if (dt > 0.05f)  dt = 0.05f;   // 限制最大间隔
//        if (dt < 0.001f) dt = 0.001f;  // 防止除零
//    }
//    last_time = now;

//    // 计算误差：传感器 L_Value[2],[3],[4] 分别是左、中、右灰度值
//		 
//    float left_error  = grayv2->L_Value[grayv2->lindex] - grayv2->L_Value[grayv2->mindex];
//    float right_error = grayv2->L_Value[grayv2->rindex] - grayv2->L_Value[grayv2->mindex];
//    float error = (left_error - right_error) / 10.0f;

//    if (power < 0) error = -error;   // 倒车时误差方向取反

//    // PD 计算
//    float proportional = grayv2->kp * error;
//    float derivative = grayv2->kd * (error - doubleMotor->pid_find_line.last_error) / dt;
//    doubleMotor->pid_find_line.last_error = error;

//    float adjustment = proportional + derivative;

//    // ---- 动态限幅：确保 adjustment 不会使任意电机超出 ±100 ----
//    float max_adjust = 100.0f - fabsf(power);  // power 可能为负，用 fabsf
//    if (max_adjust < 0.0f) max_adjust = 0.0f;   // 防御：power 已经超限时关闭调节
//    if (adjustment >  max_adjust) adjustment =  max_adjust;
//    if (adjustment < -max_adjust) adjustment = -max_adjust;

//    // 计算左右电机功率
//    float left_power  = power + adjustment;
//    float right_power = power - adjustment;

//    // 最终安全钳制（理论上不会超限，但保留以防浮点误差）
//    if (left_power  >  100.0f) left_power  =  100.0f;
//    if (left_power  < -100.0f) left_power  = -100.0f;
//    if (right_power >  100.0f) right_power =  100.0f;
//    if (right_power < -100.0f) right_power = -100.0f;

//    // 输出 PWM
//    master->motor_user_info.pwm = left_power;
//    slave->motor_user_info.pwm  = right_power;
//    master->control.current_mode = MOTOR_PWM;
//    slave->control.current_mode  = MOTOR_PWM;
//}

DEV_GRAYV2 *read_gray_v2(void *self)
{ 
	 DEV_GRAYV2 *mt = (DEV_GRAYV2*)self;
	 return mt;
}

void refsh_gray_v2(DEV_GRAYV2* mt, uint8_t* data)
{ 
		gray_packet_t pkt;
    memcpy(&pkt, data, sizeof(gray_packet_t)); 
	  #if 0
    for (int i = 0; i < 7; i++) {
        mt->L_Value[i] = pkt.values[i];
        mt->L_State[i] = pkt.states[i];
			  mt->Threshold[i] = pkt.threshold[i];
    }
	  #else
		for (int i = 0; i < 7; i++) {
				int j = 6 - i;                     // 反转索引
				mt->L_Value[j] = pkt.values[i];
				mt->L_State[j] = pkt.states[i];
				mt->Threshold[j] = pkt.threshold[i];
		}
		#endif
	  mt->is_calibrate = pkt.is_calibrate;
		mt->version = pkt.version;
		 
		mt->is_resh_flag = true;
}

DEV_GRAYV2 *create_gray_v2(uint8_t index)
{ 
 	 DEV_GRAYV2 *gray = &dev_gray_v2[index];
	 memset(gray,0,sizeof(DEV_GRAYV2));
	 gray->base.type = DEV_ID_GRAY_V2;
	 memset(gray->base.name,0,sizeof(gray->base.name));
	 strcpy(gray->base.name,"grayv2"); 
   gray->lindex =2;
	 gray->mindex = 3;
	 gray->rindex = 4;
	 return gray;	  
}

void free_gray_v2(uint8_t id)
{ 
	 DEV_GRAYV2 *gray = read_gray_v2((SensorBase *)getDevBase(id));
   if(gray!=NULL)
	 {
	    memset(gray,0,sizeof(DEV_GRAYV2));
	 }
}

 
