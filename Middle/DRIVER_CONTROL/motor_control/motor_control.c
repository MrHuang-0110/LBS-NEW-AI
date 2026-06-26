#include "motor_control.h"
#include "motor.h" 
#include "stdarg.h"
#include "portagree.h"
// 全局控制器实例
static MotorController g_controller;

// 函数声明
static void process_command_queue(void);
static void process_single_command(MotorCommand* cmd);
static void process_group_command(MotorCommand* cmd);
static void perform_safety_checks(void);
static void update_performance_stats(uint32_t cycle_start);
static void execute_motor_control(void);
static void execute_dual_motor_control(void);
static void update_motor_status(void);
static bool is_motor_in_group(uint8_t motor_id);
static float enhanced_sync_compensation(MotorSyncPair *pair);

// 默认配置
static const MotorControllerConfig DEFAULT_CONFIG = {
    .control_period = MOTOR_CONTROL_PERIOD_MS,
    .command_timeout = 1000,
    .safety_check_period = 100,
    .max_retry_count = 3,
    .enable_perf_monitor = false,
    .enable_debug_output = false
};

// ========== 单电机 默认钩子函数 ==========
static bool default_init_hook(uint8_t motor_id)
{  
    return true;
}

static void default_idle_hook(uint8_t motor_id, MotorStatus* status)
{
    status->state = MOTOR_STATE_IDLE;
}

static void default_pwm_hook(uint8_t motor_id, MotorStatus* status)
{
    MotorCommand *cmd = &status->cmd;
    int16_t pwm = cmd->params.single.pwm;
    motor_hw_set_pwm(motor_id, pwm);
  //  USB_printf("%.2f,%d\r\n",status->actual_speed,pwm);
}

static void default_speed_hook(uint8_t motor_id, MotorStatus* status)
{
    MotorCommand *cmd = &status->cmd;
    
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(motor_id));
    if(_motor_ == NULL) return;

    if(ImprovedRamp_SetTarget(&_motor_->ramp, cmd->params.single.speed,_motor_->min_speed))
        pid_control_init(&_motor_->pid_speed);

    uint32_t current_time = xTaskGetTickCount();
    float ramp_speed = ImprovedRamp_Update(&_motor_->ramp, current_time);
    
    float error = ramp_speed - status->actual_speed;
    int16_t pwm = _speed_loop(motor_id, error);
    motor_hw_set_pwm(motor_id, pwm);
  //  USB_printf("%.2f,%.2f,%d\r\n",ramp_speed,status->actual_speed,pwm);
}

static void default_speed_pos_hook(uint8_t motor_id,MotorStatus*status)
{ 
    MotorCommand *cmd = &status->cmd;   
    DEV_MOTOR *_motor_ = read_motor((SensorBase *)getDevBase(motor_id));
    if(_motor_ == NULL) return;   

    float target_position = cmd->params.single.position;
    float current_position = status->actual_position;
    float max_speed = cmd->params.single.speed;

    float direction_adjust = _pos_loop(motor_id, 
                                      (target_position - current_position),
                                       current_position); 
     
       if (fabs(direction_adjust) > 0 && fabs(direction_adjust) < _motor_->min_speed) {
           
           direction_adjust = copysign(_motor_->min_speed, direction_adjust);
       }

    int posvalue = RATE_LIMITATION(direction_adjust, 
                                    -fabs(max_speed), 
                                    fabs(max_speed));
    
    float error = posvalue - status->actual_speed;
    int16_t pwm = _speed_loop(motor_id, error);
    motor_hw_set_pwm(motor_id, pwm); 
//    USB_printf("%.2f,%.2f,%.2f,%.2f\r\n",max_speed,status->actual_speed,target_position,current_position);
}


static void default_emergency_stop_hook(uint8_t motor_id, MotorStatus* status)
{
    MotorCommand *cmd = &status->cmd;
    motor_hw_emergency_stop(motor_id, cmd->params.single.stop_mode);
    status->state = MOTOR_STATE_IDLE;
}

// ========== 双电机 默认钩子函数 ==========
static void default_dual_pwm_hook(MotorStatus *master, MotorStatus *slave)
{
    MotorCommand *master_cmd = &master->cmd;
    int16_t master_pwm = master_cmd->params.dual.pwm_master;
    int16_t slave_pwm = master_cmd->params.dual.pwm_slave;
    
    motor_hw_set_pwm(master->id, master_pwm);
    motor_hw_set_pwm(slave->id, slave_pwm);
    
}

static void default_dual_speed_hook(MotorStatus *master, MotorStatus *slave)
{
    MotorGroup *groups = motor_get_group(0);
    MotorCommand *master_cmd = &master->cmd;
    float target_speed_master = master_cmd->params.dual.speed_master;
    float target_speed_slave = master_cmd->params.dual.speed_slave;
    
    float master_actual_speed = master->actual_speed;
    float slave_actual_speed = slave->actual_speed;

    int16_t master_pwm,slave_pwm;
        

    if(target_speed_master!=target_speed_slave)
    {
        master_pwm = _speed_loop(master->id, (target_speed_master - master_actual_speed));
        slave_pwm = _speed_loop(slave->id, (target_speed_slave - slave_actual_speed)); 
        motor_hw_set_pwm(master->id, master_pwm);
        motor_hw_set_pwm(slave->id, slave_pwm);   
    }
    else
    {
        groups->syncpair.left_speed = master_actual_speed;
        groups->syncpair.right_speed = slave_actual_speed;
        groups->syncpair.speed_error = (groups->syncpair.left_speed - groups->syncpair.right_speed);
        groups->syncpair.target_speed = target_speed_master;
        if(fabsf(groups->syncpair.target_speed) < 1.0f)
        {
            target_speed_master = 0; 
            target_speed_slave = 0;
        }
        else
        {
            float compensation = enhanced_sync_compensation(&groups->syncpair);
            
            if (groups->syncpair.speed_error > 0) {
                target_speed_master = groups->syncpair.target_speed - compensation;
                target_speed_slave = groups->syncpair.target_speed + compensation;
            } else {
                target_speed_master = groups->syncpair.target_speed + compensation;
                target_speed_slave = groups->syncpair.target_speed - compensation;
            }
            
            if (groups->syncpair.target_speed > 0) {
                target_speed_master = (target_speed_master < 0) ? 0 : target_speed_master;
                target_speed_slave = (target_speed_slave < 0) ? 0 : target_speed_slave;
            } else {
                target_speed_master = (target_speed_master > 0) ? 0 : target_speed_master;
                target_speed_slave = (target_speed_slave > 0) ? 0 : target_speed_slave;
            }
            
          master_pwm = _speed_loop(master->id, (target_speed_master - master_actual_speed));
          slave_pwm = _speed_loop(slave->id, (target_speed_slave - slave_actual_speed));             
        }
        
      motor_hw_set_pwm(master->id, master_pwm);
      motor_hw_set_pwm(slave->id, slave_pwm);      
            
    }
   
}
static void default_dual_stop_hook(MotorStatus *master, MotorStatus *slave)
{
    MotorCommand *mastercmd = &master->cmd;
    MotorCommand *salvecmd = &slave->cmd;
    
    motor_hw_emergency_stop(master->id, mastercmd->params.dual.stop_mode);
    motor_hw_emergency_stop(slave->id, salvecmd->params.dual.stop_mode);
    
    master->state = MOTOR_STATE_IDLE;
    master->state = MOTOR_STATE_IDLE;
}

static void default_update_hook(uint8_t motor_id, MotorStatus* status)
{
    status->actual_speed = motor_hw_get_speed(motor_id);
    status->actual_position = motor_hw_get_position(motor_id);
    status->temperature = 0;
    status->current = 0;
    status->update_count++;
    status->last_update_time = xTaskGetTickCount();
}

static bool default_safety_hook(uint8_t motor_id, MotorCommand* cmd, MotorStatus* status)
{
    bool is_safe = true;
    
    // 检查电机ID有效性
	  
    if (motor_id >= MAX_MOTORS) {
        is_safe = false;
        g_controller.error_hook(motor_id, MOTOR_ERROR_HARDWARE_FAULT, status);
    }
    
    // 检查当前端口是否为电机
	 
    if(is_motor_base(motor_id)){ 
        is_safe = false;
        g_controller.error_hook(motor_id, MOTOR_ERROR_DISCONNECTION, status);
    }
  
    return is_safe;
}

static void default_error_hook(uint8_t motor_id, MotorErrorCode error, MotorStatus* status)
{
    status->state = MOTOR_STATE_IDLE;
    status->error_code = error;
}

static void default_debug_hook(const char* format, ...)
{  
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
//    USB_printf("%s", buffer);
}

// ========== 核心控制任务 ==========
static void motor_control_task(void *arg)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t last_safety_check = 0;
 
    while (g_controller.is_running) {
        uint32_t cycle_start = xTaskGetTickCount();
        
        // 处理命令队列
        process_command_queue();
   
        // 执行控制：双电机和单电机同时运行
       // execute_dual_motor_control();   // 双电机控制
        execute_motor_control();        // 单电机控制
			
        update_motor_status();
              
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_safety_check >= g_controller.config.safety_check_period) {
            perform_safety_checks();
            last_safety_check = current_time;
        }
             
        update_performance_stats(cycle_start);
        
        vTaskDelayUntil(&xLastWakeTime, g_controller.config.control_period);
    }
    
    motor_emergency_stop_all();
    g_controller.is_running = true;
    vTaskDelete(NULL);
}

// ========== 内部函数实现 ==========
static bool is_motor_in_group(uint8_t motor_id)
{
    for (int i = 0; i < MAX_MOTOR_GROUPS; i++) {
        MotorGroup* group = &g_controller.groups[i];
        if (group->is_active) {
            for (int j = 0; j < group->motor_count; j++) {
                if (group->motor_ids[j] == motor_id) {
                    return true;
                }
            }
        }
    }
    return false;
}
static float enhanced_sync_compensation(MotorSyncPair *pair) {

    MotorGroup *groups = motor_get_group(0);
    float base_gain = groups->syncpair.base_gain;  
    
    if (fabsf(pair->speed_error) < 2.0f) {
        return 0.0f;
    }  
    bool left_stopped = (fabsf(pair->left_speed) < 5.0f) && (fabsf(pair->target_speed) > 10.0f);
    bool right_stopped = (fabsf(pair->right_speed) < 5.0f) && (fabsf(pair->target_speed) > 10.0f);
    
    if (left_stopped || right_stopped) {
        return fabsf(pair->target_speed) * 0.8f; 
    }
    float compensation = fabsf(pair->speed_error) * base_gain;
    
    float max_compensation = fabsf(pair->target_speed) * 0.6f;  
    if (compensation > max_compensation) {
        compensation = max_compensation;
    }
    
    return compensation;
}

static void process_command_queue(void)
{
    MotorCommand cmd;
    
    while (xQueueReceive(g_controller.command_queue, &cmd, 0) == pdTRUE) {
        if (cmd.is_group_command) {
            // 处理组命令
            process_group_command(&cmd);
        } else {
            // 处理单电机命令			 
            process_single_command(&cmd);
        }
    }
}

static void process_single_command(MotorCommand* cmd)
{
	   
    if (cmd->target_id >= MAX_MOTORS) {
        return;
    }
    
    MotorStatus* status = &g_controller.motors[cmd->target_id];
    
    // 安全检查
 
    if (!g_controller.safety_hook(cmd->target_id, cmd, status)) {
        return;
    }
 
    memcpy((MotorCommand*)&status->cmd, cmd, sizeof(status->cmd));
    status->mode = cmd->mode;  
    status->state = MOTOR_STATE_RUNNING;
    status->last_update_time = xTaskGetTickCount();

}

static void process_group_command(MotorCommand* cmd)
{
    MotorGroup* group = &g_controller.groups[0];  // 只有一个组
    
    if (!group->is_active) {
        return;
    }
    
    // 设置组模式
    group->mode = cmd->mode;
    group->last_update_time = xTaskGetTickCount();
    
    // 设置组内电机的状态
    for (int i = 0; i < group->motor_count; i++) {
        uint8_t motor_id = group->motor_ids[i];
        MotorStatus* status = &g_controller.motors[motor_id];
        
        // 复制命令到每个电机状态
        memcpy((MotorCommand*)&status->cmd, cmd, sizeof(status->cmd));
        status->mode = cmd->mode;  
        status->state = MOTOR_STATE_RUNNING;
        status->last_update_time = xTaskGetTickCount();
    }
    
}

static void execute_motor_control(void)
{
    for (int i = 0; i < MAX_MOTORS; i++) {
        MotorStatus* status = &g_controller.motors[i];
        
        // 跳过在组中的电机（由双电机控制处理）
			  #if 0
        if (is_motor_in_group(i)) {
            continue;
        }
        #endif
        if (status->state == MOTOR_STATE_RUNNING) {           
            // 执行控制钩子
            MotorControlHook control_hook = g_controller.control_hooks[status->mode];
            if (control_hook != NULL) {
                control_hook(i, status);
            }
        }
    }
}

static void execute_dual_motor_control(void)
{
    for (int i = 0; i < MAX_MOTOR_GROUPS; i++) {
        MotorGroup* group = &g_controller.groups[i];
        
        // 检查组是否激活且是双电机组
        if (!group->is_active || group->motor_count != 2) {
            continue;
        }
        
        uint8_t master_id = group->motor_ids[0];
        uint8_t slave_id = group->motor_ids[1];
        
        MotorStatus* master_status = &g_controller.motors[master_id];
        MotorStatus* slave_status = &g_controller.motors[slave_id];
        
        // 检查双电机是否都处于运行状态
        if (master_status->state != MOTOR_STATE_RUNNING || 
            slave_status->state != MOTOR_STATE_RUNNING) {
            continue;
        }
        
        // 执行双电机控制钩子
        MotorDualControlHook dual_hook = g_controller.control_dual_hooks[group->mode];
        if (dual_hook != NULL) {
            dual_hook(master_status, slave_status);
        }
    }
}

static void update_motor_status(void)
{
    for (int i = 0; i < MAX_MOTORS; i++) {
        MotorStatus* status = &g_controller.motors[i];
        
        if (status->state != MOTOR_STATE_INIT) {
            // 调用状态更新钩子
            g_controller.update_hook(i, status);
            
            // 更新运行时间
            if (status->state == MOTOR_STATE_RUNNING) {
                status->run_time += g_controller.config.control_period;
            }
        }
    }
}

static void perform_safety_checks(void)
{
    for (int i = 0; i < MAX_MOTORS; i++) {
        MotorStatus* status = &g_controller.motors[i];
        
        // 跳过未初始化的电机
        if (status->state == MOTOR_STATE_INIT) {
            continue;
        }
        
        // 执行安全检查
        MotorCommand dummy_cmd = { .target_id = (uint8_t)i };
        if (!g_controller.safety_hook(i, &dummy_cmd, status)) {
            continue;
        }
    }
}

static void update_performance_stats(uint32_t cycle_start)
{
    uint32_t cycle_time = xTaskGetTickCount() - cycle_start;
    
    g_controller.stats.total_cycles++;
    g_controller.stats.total_cycle_time += cycle_time;
    
    if (cycle_time > g_controller.stats.max_cycle_time) {
        g_controller.stats.max_cycle_time = cycle_time;
    }
    
    if (cycle_time < g_controller.stats.min_cycle_time || 
        g_controller.stats.min_cycle_time == 0) {
        g_controller.stats.min_cycle_time = cycle_time;
    }
    
    // 每100周期输出性能统计
    if (g_controller.config.enable_perf_monitor && 
        (g_controller.stats.total_cycles % 100 == 0)) {
        uint32_t avg_cycle_time = g_controller.stats.total_cycle_time / 100;
        uint32_t cpu_usage = (avg_cycle_time * 100) / g_controller.config.control_period;
        
        g_controller.debug_hook("=== Motor Control Performance ===\r\n");
        g_controller.debug_hook("Cycles: %lu, Avg: %lums, Usage: %lu%%\r\n", 
                               g_controller.stats.total_cycles, avg_cycle_time, cpu_usage);
        g_controller.debug_hook("Min: %lums, Max: %lums, Errors: %lu\r\n", 
                               g_controller.stats.min_cycle_time, 
                               g_controller.stats.max_cycle_time,
                               g_controller.stats.error_count);
        
        // 重置统计
        g_controller.stats.total_cycle_time = 0;
        g_controller.stats.min_cycle_time = 0;
        g_controller.stats.max_cycle_time = 0;
    }
}

// ========== 公共API实现 ==========
bool motor_controller_init(const MotorControllerConfig* config)
{
    if (g_controller.is_initialized) {
        return true;
    }
    
    // 复制配置
    if (config != NULL) {
        g_controller.config = *config;
    } else {
        g_controller.config = DEFAULT_CONFIG;
    }
    
    // 初始化电机状态
    memset(g_controller.motors, 0, sizeof(g_controller.motors));
    for (int i = 0; i < MAX_MOTORS; i++) {
        g_controller.motors[i].id = i;
        g_controller.motors[i].state = MOTOR_STATE_INIT;
        g_controller.motors[i].mode = MOTOR_MODE_IDLE;
    }
    
    // 初始化组状态
    memset(g_controller.groups, 0, sizeof(g_controller.groups));
    
    // 初始化性能统计
    memset(&g_controller.stats, 0, sizeof(g_controller.stats));
    
    // 注册默认钩子函数
    g_controller.init_hook = default_init_hook;
    
    /*单电机钩子*/
    g_controller.control_hooks[MOTOR_MODE_IDLE] = default_idle_hook;
    g_controller.control_hooks[MOTOR_MODE_PWM_DIRECT] = default_pwm_hook;
    g_controller.control_hooks[MOTOR_MODE_SPEED_PID] = default_speed_hook;
    g_controller.control_hooks[MOTOR_MODE_POS_SPEED_PID] =default_speed_pos_hook;
    g_controller.control_hooks[MOTOR_MODE_EMERGENCY_STOP] = default_emergency_stop_hook;

    /*双电机钩子*/
    g_controller.control_dual_hooks[MOTOR_DUAL_MODE_PWM_DIRECT] = default_dual_pwm_hook;
    g_controller.control_dual_hooks[MOTOR_DUAL_MODE_SPEED_PID] = default_dual_speed_hook;
    g_controller.control_dual_hooks[MOTOR_DUAL_MODE_STOP] = default_dual_stop_hook;
    
    g_controller.update_hook = default_update_hook;
    g_controller.safety_hook = default_safety_hook;
    g_controller.error_hook = default_error_hook;
    g_controller.debug_hook = default_debug_hook;
    
    // 创建RTOS对象
    g_controller.command_queue = xQueueCreate(MOTOR_CMD_QUEUE_SIZE, sizeof(MotorCommand));
    if (g_controller.command_queue == NULL) {
        return false;
    }
    
    g_controller.mutex = xSemaphoreCreateMutex();
    if (g_controller.mutex == NULL) {
        vQueueDelete(g_controller.command_queue);
        return false;
    }
    
    g_controller.is_initialized = true;
    g_controller.is_running = false;
 
    return true;
}

bool motor_controller_start(void)
{
    if (!g_controller.is_initialized || g_controller.is_running) {
        return false;
    }
    
    // 初始化所有电机硬件
    for (int i = 0; i < MAX_MOTORS; i++) {
        if (!g_controller.init_hook(i)) {
            return false;
        }
        g_controller.motors[i].state = MOTOR_STATE_IDLE;
    }
    
    // 创建控制任务
    BaseType_t result = xTaskCreate(
        motor_control_task,
        "Motor Control",
        configMINIMAL_STACK_SIZE * 4,
        NULL,
        6,
        &g_controller.control_task
    );
    
    if (result != pdPASS) {
        return false;
    }
    
    g_controller.is_running = true;
 
    return true;
}

bool motor_controller_stop(void)
{
    if (!g_controller.is_running) {
        return true;
    }
 
    g_controller.is_running = false;
    
    #if 0
    if (g_controller.control_task != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));  // 给任务时间退出
    }
    #else
    if (!g_controller.is_running) {
        vTaskDelay(pdMS_TO_TICKS(100));  // 给任务时间退出
    }
		#endif
		
    g_controller.is_initialized = false;
    motor_group_destroy(0);
    
    if(g_controller.command_queue != NULL)
        vQueueDelete(g_controller.command_queue);
    if(g_controller.mutex != NULL)
        vSemaphoreDelete(g_controller.mutex);
    g_controller.is_running = false;
    return true;
}

bool motor_send_command(const MotorCommand* cmd)
{
    if (!g_controller.is_initialized || cmd == NULL) {
        return false;
    }
    
    // 添加时间戳
    MotorCommand timestamped_cmd = *cmd;
    timestamped_cmd.timestamp = xTaskGetTickCount();
    
    if(!g_controller.is_running)return false;
    BaseType_t result = xQueueSend(g_controller.command_queue, &timestamped_cmd, portMAX_DELAY);
    return (result == pdTRUE);
}

bool motor_send_command_urgent(const MotorCommand* cmd)
{
    if (!g_controller.is_initialized || cmd == NULL) {
        return false;
    }
    
    MotorCommand timestamped_cmd = *cmd;
    timestamped_cmd.timestamp = xTaskGetTickCount();
    
    BaseType_t result = xQueueSendToFront(g_controller.command_queue, &timestamped_cmd, portMAX_DELAY);
    return (result == pdTRUE);
}

// 钩子注册函数
void motor_register_init_hook(MotorInitHook hook)
{
    if (hook != NULL) {
        g_controller.init_hook = hook;
    }
}

void motor_register_control_hook(MotorControlMode mode, MotorControlHook hook)
{
    if (mode < MAX_MODES && hook != NULL) {
        g_controller.control_hooks[mode] = hook;
    }
}

void motor_register_dual_control_hook(MotorDualMode mode, MotorDualControlHook hook)
{
    if (mode < MAX_DUAL_MODES && hook != NULL) {
        g_controller.control_dual_hooks[mode] = hook;
    }
}

void motor_register_update_hook(MotorUpdateHook hook)
{
    if (hook != NULL) {
        g_controller.update_hook = hook;
    }
}

void motor_register_safety_hook(MotorSafetyHook hook)
{
    if (hook != NULL) {
        g_controller.safety_hook = hook;
    }
}

void motor_register_error_hook(MotorErrorHook hook)
{
    if (hook != NULL) {
        g_controller.error_hook = hook;
    }
}

void motor_register_debug_hook(MotorDebugHook hook)
{
    if (hook != NULL) {
        g_controller.debug_hook = hook;
    }
}

MotorStatus* motor_get_status(uint8_t motor_id)
{
    if (motor_id >= MAX_MOTORS) {
        return NULL;
    }
    return &g_controller.motors[motor_id];
}

MotorGroup* motor_get_group(uint8_t group_id)
{
    if (group_id >= MAX_MOTOR_GROUPS) {
        return NULL;
    }
    return &g_controller.groups[group_id];
}

PerformanceStats* motor_get_performance_stats(void)
{
    return &g_controller.stats;
}

bool motor_group_create(uint8_t group_id, const uint8_t* motor_ids, uint8_t count)
{
    if (group_id >= MAX_MOTOR_GROUPS || count != 2) {  // 只能创建2电机组
        return false;
    }
    
    xSemaphoreTake(g_controller.mutex, portMAX_DELAY);
    
    MotorGroup* group = &g_controller.groups[group_id];
    group->group_id = group_id;
    group->motor_count = count;
    memcpy(group->motor_ids, motor_ids, count);
    group->is_active = true;
    group->last_update_time = xTaskGetTickCount();
    
    DEV_MOTOR *master = get_motor_base(group->motor_ids[0]);
    DEV_MOTOR *slave = get_motor_base(group->motor_ids[1]);
    
    if(master != NULL && slave != NULL) { 
        group->speedm = master->max_speed / 2.0f;
        group->speeds = slave->max_speed / 2.0f;
        group->dspeed = (master->max_speed + slave->max_speed) / 2.0f;
    } else { 
        group->speedm = 50;
        group->speeds = 50;
        group->dspeed = 50;
    }
    group->stopmodes = STOP_MODE_SLIDE; 
    
    /*偏航角PID*/
    group->yaw_kp = 2.5f;group->yaw_ki = 0.5f;group->yaw_kd = 0.5f;
    /*旋转PID*/
    group->angle_kp = 2.5f;group->angle_ki = 0.1f;group->angle_kd = 0.1f;

    /*电机同步系数*/
    group->syncpair.base_gain =0.3f;
    extern void line_compensation_init(float kp,float ki,float kd);
    line_compensation_init(0,0,0);
    xSemaphoreGive(g_controller.mutex);

    
    return true;
}

bool motor_group_destroy(uint8_t group_id)
{
    if (group_id >= MAX_MOTOR_GROUPS) {
        return false;
    }
    
    xSemaphoreTake(g_controller.mutex, portMAX_DELAY);
    
    MotorGroup* group = &g_controller.groups[group_id];
    group->is_active = false;
    group->motor_count = 0;
    
    xSemaphoreGive(g_controller.mutex);
    
    
    return true;
}

void motor_emergency_stop(uint8_t motor_id, MotorStopMode stop_mode)
{
    MotorCommand cmd = {
        .target_id = motor_id,
        .mode = MOTOR_MODE_EMERGENCY_STOP,
        .is_group_command = false,
        .params.single.stop_mode = stop_mode
    };
    motor_send_command_urgent(&cmd);
}

void motor_dual_emergency_stop(uint8_t group_id, MotorStopMode stop_mode)
{
    MotorGroup* group = motor_get_group(group_id);
    if (group == NULL || !group->is_active) {
        return;
    }
    
    for (int i = 0; i < group->motor_count; i++) {
        motor_emergency_stop(group->motor_ids[i], stop_mode);
    }
}

void motor_emergency_stop_all(void)
{
    // 停止所有单电机
    for(uint8_t i = 0; i < MAX_MOTORS; i++) {
        if(is_motor_base(i))
            continue;  
//				USB_printf("->close motor[%d]\r\n",i);
        motor_hw_emergency_stop(i, STOP_MODE_BRAKE);
    }
    
    vTaskDelay(200);

    // 释放所有电机
    for(uint8_t i = 0; i < MAX_MOTORS; i++) {
        if(is_motor_base(i))
            continue;
        motor_hw_emergency_stop(i, STOP_MODE_SLIDE);
        pid_control_init(&get_motor_base(i)->pid_speed);
        pid_control_init(&get_motor_base(i)->pid_pos);
 
				ImprovedRamp_Init(&get_motor_base(i)->ramp,get_motor_base(i)->min_speed,1.0f,1.0f);
    }     
    vTaskDelay(200);
}

// 工具函数
const char* motor_mode_to_string(MotorControlMode mode)
{
    static const char* strings[] = {
        "IDLE", "PWM_DIRECT", "SPEED_PID", "POSITION_PID", 
        "SYNC_SPEED", "DIFF_DRIVE", "TURN", "STOP", 
        "EMERGENCY_STOP", "CUSTOM"
    };
    
    if (mode <= MOTOR_MODE_CUSTOM) {
        return strings[mode];
    }
    return "UNKNOWN";
}

const char* motor_dual_mode_to_string(MotorDualMode mode)
{
    static const char* strings[] = {
        "DUAL_PWM", "DUAL_SPEED", "SYNC_SPEED"
    };
    
    if (mode < MAX_DUAL_MODES) {
        return strings[mode];
    }
    return "UNKNOWN";
}

const char* motor_state_to_string(MotorState state)
{
    static const char* strings[] = {
        "INIT", "IDLE", "RUNNING", "STOPPING", "ERROR", "FAULT"
    };
    
    if (state <= MOTOR_STATE_FAULT) {
        return strings[state];
    }
    return "UNKNOWN";
}

const char* motor_error_to_string(MotorErrorCode error)
{
    static const char* strings[] = {
        "NONE", "OVER_CURRENT", "OVER_SPEED", "OVER_TEMP", 
        "COMMUNICATION", "HARDWARE_FAULT"
    };
    
    if (error <= MOTOR_ERROR_HARDWARE_FAULT) {
        return strings[error];
    }
    return "UNKNOWN";
}

void create_motor_control(void)
{ 
    motor_controller_init(&DEFAULT_CONFIG);
    motor_controller_start();
}
