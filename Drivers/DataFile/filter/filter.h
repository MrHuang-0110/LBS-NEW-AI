#ifndef __FILTER_H
#define __FILTER_H
#include "sys.h"


// 自适应滤波器
typedef struct {
    float min_alpha;    // 最小滤波系数
    float max_alpha;    // 最大滤波系数
    float speed_threshold; // 速度阈值
    float prev_output;  // 上一次输出
} AdaptiveFilter;
 
typedef struct {
 
    // 同步控制参数
    float sync_gain;           // 同步增益
    float max_sync_adjustment; // 最大同步调整量
    float speed_diff_threshold;// 速度差阈值，低于此值不进行补偿
    
    // 状态变量
    float left_speed;
    float right_speed;
    float speed_difference;
    float sync_adjustment;
    
    // 滤波参数
    float diff_filter_alpha;   // 速度差滤波系数
    float filtered_speed_diff; // 滤波后的速度差
}DualMotorSyncController;

void init_adaptive_filter(AdaptiveFilter* filter, float min_alpha, float max_alpha, float speed_threshold);
float adaptive_filter(AdaptiveFilter* filter, float input, float current_speed); 
float calculate_sync_adjustment(DualMotorSyncController* sync);
float calculate_filtered_speed_diff(DualMotorSyncController* sync,float left_speed, float right_speed,float dt);
void init_dual_motor_sync(DualMotorSyncController* sync, float sync_gain, float max_adjustment, float diff_threshold, float filter_alpha);
#endif
