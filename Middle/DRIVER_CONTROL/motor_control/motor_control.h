#ifndef __MOTOR_CONTROLLER_H__
#define __MOTOR_CONTROLLER_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"   
 
// 系统配置
#define MAX_MOTORS               8
#define MOTOR_CONTROL_PERIOD_MS  5
#define MOTOR_CONTROL_PY_RERIOD_MS 1
#define MOTOR_CMD_QUEUE_SIZE     512
#define MAX_MOTOR_GROUPS         1
#define MAX_GROUP_MOTORS         2  // 每组最多2个电机

#define MOTOR_MAX_PWM    99
#define MOTOR_MAX_SPEED  0xFFFF
#define MOTOR_MAX_TEMP   0xFFFF

// 电机控制模式
typedef enum {
    MOTOR_MODE_IDLE = 0,           // 空闲模式
    MOTOR_MODE_PWM_DIRECT,         // PWM直接控制
    MOTOR_MODE_SPEED_PID,          // 速度PID控制
    MOTOR_MODE_POS_SPEED_PID,      // 速度和位置串联控制
    MOTOR_MODE_EMERGENCY_STOP,     // 紧急停止
    MOTOR_MODE_CUSTOM,             // 自定义模式
    MAX_MODES                      // 模式总数
} MotorControlMode;

typedef enum {
    MOTOR_DUAL_MODE_PWM_DIRECT = 0,
    MOTOR_DUAL_MODE_SPEED_PID,
    MOTOR_DUAL_MODE_STOP,
    MAX_DUAL_MODES
} MotorDualMode;

// 停止模式
typedef enum {
    STOP_MODE_SLIDE = 0,           // 滑行停止
    STOP_MODE_BRAKE,               // 刹车停止
    STOP_MODE_HOLD                 // 保持位置停止
} MotorStopMode;

typedef enum{

  LOW_SPEDD = 0,
  MIDDLE_SPEED,
  HIGHT_SPEED
} MotorAcceleration;

// 电机状态
typedef enum {
    MOTOR_STATE_INIT = 0,          // 初始化
    MOTOR_STATE_IDLE,              // 空闲
    MOTOR_STATE_RUNNING,           // 运行中
    MOTOR_STATE_STOPPING,          // 停止中
    MOTOR_STATE_ERROR,             // 错误
    MOTOR_STATE_FAULT              // 故障
} MotorState;

// 错误代码
typedef enum {
    MOTOR_ERROR_NONE = 0,          // 无错误
    MOTOR_ERROR_OVER_CURRENT,      // 过流
    MOTOR_ERROR_OVER_SPEED,        // 超速
    MOTOR_ERROR_OVER_TEMP,         // 过热
    MOTOR_ERROR_COMMUNICATION,     // 通信错误
    MOTOR_ERROR_HARDWARE_FAULT,    // 硬件故障
    MOTOR_ERROR_DISCONNECTION      // 断开连接
} MotorErrorCode;

// 单电机命令参数
typedef struct {
    int16_t pwm;                   // PWM输出值
    float speed;                   // 目标速度
    int32_t position;              // 目标位置
    MotorStopMode stop_mode;       // 停止模式
} SingleMotorParams;

// 双电机命令参数
typedef struct {
    uint8_t motor_ids[2];          // 电机ID数组
    int16_t pwm_master;            // 主电机PWM
    int16_t pwm_slave;             // 从电机PWM
    float speed_master;            // 主电机速度
    float speed_slave;             // 从电机速度
    float sync_speed;              // 同步速度
    MotorStopMode stop_mode;
} DualMotorParams;

// 电机命令结构
typedef struct {
    uint8_t target_id;             // 目标电机ID或组ID
    uint8_t mode;                  // 控制模式（单电机或双电机模式）
    bool is_group_command;         // 是否为组命令
    uint32_t timestamp;            // 时间戳
    
    union {
        SingleMotorParams single;  // 单电机参数
        DualMotorParams dual;      // 双电机参数
        void* custom_data;         // 自定义数据
    } params;
} MotorCommand;

// 电机状态信息
typedef struct {
    uint8_t id;                    // 电机ID
    MotorState state;              // 当前状态
    uint8_t mode;                  // 当前模式
    MotorErrorCode error_code;     // 错误代码
    
    MotorCommand cmd;
    // 实际值
    float actual_speed;
    int32_t actual_position;
    int16_t actual_pwm;
    float temperature;
    float current;
    
    // 统计信息
    uint32_t run_time;             // 运行时间
    uint32_t update_count;         // 更新次数
    uint32_t last_update_time;     // 最后更新时间
    
    // 自定义数据
    void* custom_data;
} MotorStatus;
typedef struct{ 
    float speed_error;
    float left_speed,right_speed;
    float target_speed;
    float base_gain;
}MotorSyncPair;


// 电机组信息
typedef struct {
    uint8_t group_id;
    uint8_t motor_ids[MAX_GROUP_MOTORS];
    uint8_t motor_count;
    uint8_t mode;
    bool is_active;
    uint32_t last_update_time;
    
    // 组参数
    float speedm;        // 主电机速度
    float speeds;        // 从电机速度  
    float dspeed;        // 差分速度
    MotorStopMode stopmodes; // 停止模式
    
    MotorSyncPair syncpair;
    
    float yaw_kp,yaw_ki,yaw_kd;
    float angle_kp,angle_ki,angle_kd;
    float line_kp,line_ki,line_kd;
    //PID_Controller pid_line;
} MotorGroup;


typedef struct {
    uint32_t total_cycles;         
    uint32_t max_cycle_time;       
    uint32_t min_cycle_time;       
    uint32_t total_cycle_time;     
    uint32_t error_count;          
    uint32_t timeout_count;        
} PerformanceStats;


typedef bool (*MotorInitHook)(uint8_t motor_id);
typedef void (*MotorControlHook)(uint8_t motor_id, MotorStatus* status);
typedef void (*MotorDualControlHook)(MotorStatus *master, MotorStatus *slave);
typedef void (*MotorUpdateHook)(uint8_t motor_id, MotorStatus* status);
typedef bool (*MotorSafetyHook)(uint8_t motor_id, MotorCommand *cmd, MotorStatus* status);
typedef void (*MotorErrorHook)(uint8_t motor_id, MotorErrorCode error, MotorStatus* status);
typedef void (*MotorDebugHook)(const char* format, ...);


typedef struct {
    uint32_t control_period;       
    uint16_t command_timeout;     
    uint16_t safety_check_period;  
    uint8_t max_retry_count;       
    bool enable_perf_monitor;      
    bool enable_debug_output;      
} MotorControllerConfig;


typedef struct {
    MotorControllerConfig config;
    MotorStatus motors[MAX_MOTORS];
    MotorGroup groups[MAX_MOTOR_GROUPS];
    PerformanceStats stats;
    
    QueueHandle_t command_queue;
    SemaphoreHandle_t mutex;
    TaskHandle_t control_task;
    
    bool is_initialized;
    bool is_running;
    
    MotorInitHook init_hook;
    MotorControlHook control_hooks[MAX_MODES];
    MotorDualControlHook control_dual_hooks[MAX_DUAL_MODES];
    MotorUpdateHook update_hook;
    MotorSafetyHook safety_hook;
    MotorErrorHook error_hook;
    MotorDebugHook debug_hook;
} MotorController;


bool motor_controller_init(const MotorControllerConfig* config);


bool motor_controller_start(void);
bool motor_controller_stop(void);

bool motor_send_command(const MotorCommand* cmd);
bool motor_send_command_urgent(const MotorCommand* cmd);


void motor_register_init_hook(MotorInitHook hook);
void motor_register_control_hook(MotorControlMode mode, MotorControlHook hook);
void motor_register_dual_control_hook(MotorDualMode mode, MotorDualControlHook hook);
void motor_register_update_hook(MotorUpdateHook hook);
void motor_register_safety_hook(MotorSafetyHook hook);
void motor_register_error_hook(MotorErrorHook hook);
void motor_register_debug_hook(MotorDebugHook hook);

MotorStatus* motor_get_status(uint8_t motor_id);
MotorGroup* motor_get_group(uint8_t group_id);
PerformanceStats* motor_get_performance_stats(void);

bool motor_group_create(uint8_t group_id, const uint8_t* motor_ids, uint8_t count);
bool motor_group_destroy(uint8_t group_id);

void motor_emergency_stop(uint8_t motor_id, MotorStopMode stop_mode);
void motor_dual_emergency_stop(uint8_t group_id, MotorStopMode stop_mode);
void motor_emergency_stop_all(void);

const char* motor_mode_to_string(MotorControlMode mode);
const char* motor_dual_mode_to_string(MotorDualMode mode);
const char* motor_state_to_string(MotorState state);
const char* motor_error_to_string(MotorErrorCode error);

void create_motor_control(void);

#endif
