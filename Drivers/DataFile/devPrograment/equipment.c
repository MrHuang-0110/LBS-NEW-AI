#include "equipment.h"
#include "motor.h"

void timer_callback_wrapper(TimerHandle_t xTimer)
{
// 获取关联的TimerCommandPair
    TimerCommandPair *pair = (TimerCommandPair*)pvTimerGetTimerID(xTimer);
    if (!pair || !pair->cmd) return;
    
    // 解析命令头
    CommandHeader *header = (CommandHeader*)pair->cmd;
    
    // 根据设备类型处理命令
    switch (header->device_type) {
        case DEV_ID_BIG_MOTOR:
        case DEV_ID_SMALL_Motor:
            handle_servo_command((ServoCommandData*)pair->cmd);
            break;
            
        case DEVICE_TYPE_SENSOR:
            handle_sensor_command((SensorCommandData*)pair->cmd);
            break;
            
        case DEVICE_TYPE_LED:
            handle_led_command((LedCommandData*)pair->cmd);
            break;
            
        default:
            // 未知设备类型
            USB_printf("Unknown device type: %d\n", header->device_type);
            break;
    }
    
    // 清理资源
    vPortFree(pair->cmd);
    xTimerDelete(xTimer, 0);
    vPortFree(pair);
}

// 创建定时器并传递命令
TimerCommandPair* create_timer_with_cmd(void *cmd_data, uint32_t delay_ms)
{
    if (!cmd_data) return NULL;
    
    // 获取设备ID用于定时器命名
    CommandHeader *header = (CommandHeader*)cmd_data;
    char timer_name[20];
    snprintf(timer_name, sizeof(timer_name), "DevTmr%d_%d", 
             header->device_type, header->device_id);
    
    // 分配TimerCommandPair
    TimerCommandPair *pair = pvPortMalloc(sizeof(TimerCommandPair));
    if (!pair) {
        vPortFree(cmd_data);
        return NULL;
    }
    
    // 创建定时器
    pair->timer = xTimerCreate(
        timer_name,
        pdMS_TO_TICKS(delay_ms),
        pdFALSE,        // 单次定时器
        (void*)pair,     // 传递TimerCommandPair自身
        timer_callback_wrapper   // 统一回调函数
    );
    
    if (!pair->timer) {
        vPortFree(pair);
        vPortFree(cmd_data);
        return NULL;
    }
    
    // 关联命令数据
    pair->cmd = cmd_data;
    
    return pair;
}

 
 
 