#include "filter.h"


// 初始化自适应滤波器
void init_adaptive_filter(AdaptiveFilter* filter, float min_alpha, float max_alpha, float speed_threshold) {
    filter->min_alpha = min_alpha;
    filter->max_alpha = max_alpha;
    filter->speed_threshold = speed_threshold;
    filter->prev_output = 0;
}

// 自适应滤波函数
float adaptive_filter(AdaptiveFilter* filter, float input, float current_speed) {
    // 根据速度动态调整滤波系数
    float alpha;
    if(fabs(current_speed) < filter->speed_threshold) {
        // 低速时使用较强滤波
        alpha = filter->min_alpha;
    } else {
        // 高速时使用较弱滤波
        alpha = filter->max_alpha;
    }
    
    // 应用滤波
    filter->prev_output = alpha * input + (1 - alpha) * filter->prev_output;
    return filter->prev_output;
}

 
float calculate_filtered_speed_diff(DualMotorSyncController* sync,float left_speed, float right_speed,float dt) {
    // 更新速度值
    sync->left_speed = left_speed;
    sync->right_speed = right_speed;
    
    // 计算原始速度差
    float raw_diff = left_speed - right_speed;
    
    // 应用低通滤波减少噪声影响
    sync->filtered_speed_diff = sync->diff_filter_alpha * raw_diff + 
                               (1 - sync->diff_filter_alpha) * sync->filtered_speed_diff;
    
    sync->speed_difference = sync->filtered_speed_diff;
    
    return sync->speed_difference;
}

float calculate_sync_adjustment(DualMotorSyncController* sync) {
    float abs_diff = fabs(sync->speed_difference);
    
    // 如果速度差低于阈值，不进行补偿
    if (abs_diff < sync->speed_diff_threshold) {
        sync->sync_adjustment = 0;
        return 0;
    }
    
    // 计算补偿量（带符号）
    float adjustment = sync->sync_gain * sync->speed_difference;
    
    // 限制补偿量范围
    if (adjustment > sync->max_sync_adjustment) {
        adjustment = sync->max_sync_adjustment;
    } else if (adjustment < -sync->max_sync_adjustment) {
        adjustment = -sync->max_sync_adjustment;
    }
    
    sync->sync_adjustment = adjustment;
    
    return adjustment;
}

void init_dual_motor_sync(DualMotorSyncController* sync, 
                         float sync_gain, float max_adjustment, 
                         float diff_threshold, float filter_alpha) {
    // 初始化同步参数
    sync->sync_gain = sync_gain;
    sync->max_sync_adjustment = max_adjustment;
    sync->speed_diff_threshold = diff_threshold;
    sync->diff_filter_alpha = filter_alpha;
    
    // 初始化状态变量
    sync->left_speed = 0;
    sync->right_speed = 0;
    sync->speed_difference = 0;
    sync->filtered_speed_diff = 0;
    sync->sync_adjustment = 0; 
}
 
