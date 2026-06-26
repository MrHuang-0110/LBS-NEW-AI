#include "pid_control.h"
#include <math.h>
#include <string.h>


void PI_Init(PIController *pi, float Kp, float Ki, float out_lim, float int_lim) {
    pi->Kp = Kp;
    pi->Ki = Ki;
    pi->out_limit = out_lim;
    pi->integral_limit = int_lim;
    pi->integral = 0.0f;
    pi->last_error = 0.0f;
}
 
 
float PI_Compute(PIController *pi, float setpoint, float measurement, float dt) {
    float error = setpoint - measurement;

    // 比例项
    float P = pi->Kp * error;

    // 积分项（先累加）
    pi->integral += error * dt;

    // 积分限幅
    if (pi->integral > pi->integral_limit) pi->integral = pi->integral_limit;
    if (pi->integral < -pi->integral_limit) pi->integral = -pi->integral_limit;

    float I = pi->Ki * pi->integral;

    // 总输出
    float output = P + I;

    // 输出限幅
    if (output > pi->out_limit) {
        output = pi->out_limit;
        // 抗积分饱和：如果输出饱和且误差方向相同，撤销本次积分累加
        if (error * output > 0) {
            pi->integral -= error * dt;
        }
    } else if (output < -pi->out_limit) {
        output = -pi->out_limit;
        if (error * output > 0) {
            pi->integral -= error * dt;
        }
    }

    return output;
}
void Pos_Init(PositionController *pos, float Kp, float Ki, float out_lim, float int_lim) {
    pos->Kp = Kp;
    pos->Ki = Ki;
    pos->out_limit = out_lim;
    pos->integral_limit = int_lim;
    pos->integral = 0.0f;
    pos->target_pos = 0;
}

float Pos_Compute(PositionController *pos, float current_pos, float dt) {
    float error = pos->target_pos - current_pos;

    // 比例项
    float P = pos->Kp * error;

    // 积分项（带抗积分饱和）
    pos->integral += error * dt;
    if (pos->integral > pos->integral_limit) pos->integral = pos->integral_limit;
    if (pos->integral < -pos->integral_limit) pos->integral = -pos->integral_limit;
    float I = pos->Ki * pos->integral;

    // 期望速度
    float desired_speed = P + I;

    // 输出限幅（防止速度指令过大）
    if (desired_speed > pos->out_limit) {
        desired_speed = pos->out_limit;
        // 抗积分饱和
        if (error * desired_speed > 0) pos->integral -= error * dt;
    } else if (desired_speed < -pos->out_limit) {
        desired_speed = -pos->out_limit;
        if (error * desired_speed > 0) pos->integral -= error * dt;
    }

    return desired_speed;
}

void Pos_SetTarget(PositionController *pos, float target_pulses) {
    pos->target_pos = target_pulses;
    pos->integral = 0.0f;   // 可选：改变目标时复位积分
}

void PI_Reset(PIController *pi) {
    pi->integral = 0.0f;
}
