#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

 

typedef struct {
    float Kp, Ki;               // 比例、积分系数
    float integral;             // 积分项
    float out_limit;            // 输出限幅（绝对值）
    float integral_limit;       // 积分限幅（防止积分过深）
    float last_error;
	  float pid_prev_time;
}PIController; 

typedef struct {
    float Kp, Ki,Kd;               // 比例、积分系数
    float integral,last_error;             // 积分项
    float out_limit;            // 输出速度限幅（转/秒）
    float integral_limit;       // 积分限幅
    float target_pos;           // 目标位置（脉冲数）	  
} PositionController;

 

 
void  Pos_Init(PositionController *pos, float Kp, float Ki, float out_lim, float int_lim);
float Pos_Compute(PositionController *pos, float current_pos, float dt);
void  Pos_SetTarget(PositionController *pos, float target_pulses);
							
 
void  PI_Init(PIController *pi, float Kp, float Ki, float out_lim, float int_lim);
float PI_Compute(PIController *pi, float setpoint, float measurement, float dt);
void  PI_Reset(PIController *pi); 
 
#endif
