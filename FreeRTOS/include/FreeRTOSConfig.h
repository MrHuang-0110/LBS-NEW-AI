#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


#include "stdio.h"
extern uint32_t SystemCoreClock;
extern void PreSleepProcessing(uint32_t ulExpectedIdleTime);
extern void PostSleepProcessing(uint32_t ulExpectedIdleTime);

/* 基础配置�?*/
#define configUSE_PREEMPTION                            1                       /* 1: 抢占式调度器, 0: 协程式调度器, 无默认需定义 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         1                       /* 1: 使用硬件计算下一个要运行的任�? 0: 使用软件算法计算下一个要运行的任�? 默认: 0 */
#define configUSE_TICKLESS_IDLE                         0                       /* 1: 使能tickless低功耗模�? 默认: 0 */
#define configCPU_CLOCK_HZ                              SystemCoreClock         /* 定义CPU主频, 单位: Hz, 无默认需定义 */
//#define configSYSTICK_CLOCK_HZ                          (configCPU_CLOCK_HZ / 8)/* 定义SysTick时钟频率，当SysTick时钟频率与内核时钟频率不同时才可以定�? 单位: Hz, 默认: 不定�?*/
#define configTICK_RATE_HZ                              1000                    /* 定义系统时钟节拍频率, 单位: Hz, 无默认需定义 */
#define configMAX_PRIORITIES                            10                      /* 定义最大优先级�? 最大优先级=configMAX_PRIORITIES-1, 无默认需定义 */
#define configMINIMAL_STACK_SIZE                        256                     /* 定义空闲任务的栈空间大小, 单位: Word, 无默认需定义 */
#define configMAX_TASK_NAME_LEN                         32                      /* 定义任务名最大字符数, 默认: 16 */
#define configUSE_16_BIT_TICKS                          0                       /* 1: 定义系统时钟节拍计数器的数据类型�?6位无符号�? 无默认需定义 */
#define configIDLE_SHOULD_YIELD                         1                       /* 1: 使能在抢占式调度�?同优先级的任务能抢占空闲任务, 默认: 1 */
#define configUSE_TASK_NOTIFICATIONS                    1                       /* 1: 使能任务间直接的消息传�?包括信号量、事件标志组和消息邮�? 默认: 1 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES           1                       /* 定义任务通知数组的大�? 默认: 1 */
#define configUSE_MUTEXES                               1                       /* 1: 使能互斥信号�? 默认: 0 */
#define configUSE_RECURSIVE_MUTEXES                     1                       /* 1: 使能递归互斥信号�? 默认: 0 */
#define configUSE_COUNTING_SEMAPHORES                   1                       /* 1: 使能计数信号�? 默认: 0 */
#define configUSE_ALTERNATIVE_API                       0                       /* 已弃�?!! */
#define configQUEUE_REGISTRY_SIZE                       8                       /* 定义可以注册的信号量和消息队列的个数, 默认: 0 */
#define configUSE_QUEUE_SETS                            1                       /* 1: 使能队列�? 默认: 0 */
#define configUSE_TIME_SLICING                          1                       /* 1: 使能时间片调�? 默认: 1 */
#define configUSE_NEWLIB_REENTRANT                      0                       /* 1: 任务创建时分配Newlib的重入结构体, 默认: 0 */
#define configENABLE_BACKWARD_COMPATIBILITY             0                       /* 1: 使能兼容老版�? 默认: 1 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS         0                       /* 定义线程本地存储指针的个�? 默认: 0 */
#define configSTACK_DEPTH_TYPE                          uint16_t                /* 定义任务堆栈深度的数据类�? 默认: uint16_t */
#define configMESSAGE_BUFFER_LENGTH_TYPE                size_t                  /* 定义消息缓冲区中消息长度的数据类�? 默认: size_t */
#define configPRE_SLEEP_PROCESSING(x)       						PreSleepProcessing(x)
#define configPOST_SLEEP_PROCESSING(x)									PostSleepProcessing(x)

/* 内存分配相关定义 */
#define configSUPPORT_STATIC_ALLOCATION                 0                       /* 1: 支持静态申请内�? 默认: 0 */
#define configSUPPORT_DYNAMIC_ALLOCATION                1                       /* 1: 支持动态申请内�? 默认: 1 */
#define configTOTAL_HEAP_SIZE                           ((size_t)(200 * 1024))   /* FreeRTOS堆中可用的RAM总量, 单位: Byte, 无默认需定义 */
#define configAPPLICATION_ALLOCATED_HEAP                0                       /* 1: 用户手动分配FreeRTOS内存�?ucHeap), 默认: 0 */
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP       0                       /* 1: 用户自行实现任务创建时使用的内存申请与释放函�? 默认: 0 */

/* 钩子函数相关定义 */
#define configUSE_IDLE_HOOK                             0                       /* 1: 使能空闲任务钩子函数, 无默认需定义  */
#define configUSE_TICK_HOOK                             0                       /* 1: 使能系统时钟节拍中断钩子函数, 无默认需定义 */
#define configCHECK_FOR_STACK_OVERFLOW                  2                       /* 1: 使能栈溢出检测方�?, 2: 使能栈溢出检测方�?, 默认: 0 */
#define configUSE_MALLOC_FAILED_HOOK                    0                       /* 1: 使能动态内存申请失败钩子函�? 默认: 0 */
#define configUSE_DAEMON_TASK_STARTUP_HOOK              0                       /* 1: 使能定时器服务任务首次执行前的钩子函�? 默认: 0 */

/* 运行时间和任务状态统计相关定�?*/
#define configGENERATE_RUN_TIME_STATS                   1                       /* 1: 使能任务运行时间统计功能, 默认: 0 */
#if configGENERATE_RUN_TIME_STATS
extern void ConfigureTimeForRunTimeStats(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()        ConfigureTimeForRunTimeStats()
extern unsigned long GetRunTimeCounterValue(void);
#define portGET_RUN_TIME_COUNTER_VALUE()								GetRunTimeCounterValue()

#endif
#define configUSE_TRACE_FACILITY                        1                       /* 1: 使能可视化跟踪调�? 默认: 0 */
#define configUSE_STATS_FORMATTING_FUNCTIONS            1                       /* 1: configUSE_TRACE_FACILITY�?时，会编译vTaskList()和vTaskGetRunTimeStats()函数, 默认: 0 */

/* 协程相关定义 */
#define configUSE_CO_ROUTINES                           0                       /* 1: 启用协程, 默认: 0 */
#define configMAX_CO_ROUTINE_PRIORITIES                 2                       /* 定义协程的最大优先级, 最大优先级=configMAX_CO_ROUTINE_PRIORITIES-1, 无默认configUSE_CO_ROUTINES�?时需定义 */

/* 软件定时器相关定�?*/
#define configUSE_TIMERS                                1                               /* 1: 使能软件定时�? 默认: 0 */
#define configTIMER_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )    /* 定义软件定时器任务的优先�? 无默认configUSE_TIMERS�?时需定义 */
#define configTIMER_QUEUE_LENGTH                        20                               /* 定义软件定时器命令队列的长度, 无默认configUSE_TIMERS�?时需定义 */
#define configTIMER_TASK_STACK_DEPTH                    ( configMINIMAL_STACK_SIZE * 2) /* 定义软件定时器任务的栈空间大�? 无默认configUSE_TIMERS�?时需定义 */

/* 可选函�? 1: 使能 */
#define INCLUDE_vTaskPrioritySet                        1                       /* 设置任务优先�?*/
#define INCLUDE_uxTaskPriorityGet                       1                       /* 获取任务优先�?*/
#define INCLUDE_vTaskDelete                             1                       /* 删除任务 */
#define INCLUDE_vTaskSuspend                            1                       /* 挂起任务 */
#define INCLUDE_xResumeFromISR                          1                       /* 恢复在中断中挂起的任�?*/
#define INCLUDE_vTaskDelayUntil                         1                       /* 任务绝对延时 */
#define INCLUDE_vTaskDelay                              1                       /* 任务延时 */
#define INCLUDE_xTaskGetSchedulerState                  1                       /* 获取任务调度器状�?*/
#define INCLUDE_xTaskGetCurrentTaskHandle               1                       /* 获取当前任务的任务句�?*/
#define INCLUDE_uxTaskGetStackHighWaterMark             1                       /* 获取任务堆栈历史剩余最小�?*/
#define INCLUDE_xTaskGetIdleTaskHandle                  1                       /* 获取空闲任务的任务句�?*/
#define INCLUDE_eTaskGetState                           1                       /* 获取任务状�?*/
#define INCLUDE_xEventGroupSetBitFromISR                1                       /* 在中断中设置事件标志�?*/
#define INCLUDE_xTimerPendFunctionCall                  1                       /* 将函数的执行挂到定时器服务任�?*/
#define INCLUDE_xTaskAbortDelay                         1                       /* 中断任务延时 */
#define INCLUDE_xTaskGetHandle                          1                       /* 通过任务名获取任务句�?*/
#define INCLUDE_xTaskResumeFromISR                      1                       /* 恢复在中断中挂起的任�?*/

/* 中断嵌套行为配置 */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15                  /* 中断最低优先级 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5                   /* FreeRTOS可管理的最高中断优先级 */
#define configKERNEL_INTERRUPT_PRIORITY                 ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_API_CALL_INTERRUPT_PRIORITY           configMAX_SYSCALL_INTERRUPT_PRIORITY

/* FreeRTOS中断服务函数相关定义 */
#define xPortPendSVHandler                              PendSV_Handler
#define vPortSVCHandler                                 SVC_Handler

/* 断言 */
#define vAssertCalled(char, int) printf("Error: %s, %d\r\n", char, int)
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )

/* FreeRTOS MPU 特殊定义 */
//#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0
//#define configTOTAL_MPU_REGIONS                                8
//#define configTEX_S_C_B_FLASH                                  0x07UL
//#define configTEX_S_C_B_SRAM                                   0x07UL
//#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY            1
//#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS             1

/* ARMv8-M 安全侧端口相关定义�?*/
//#define secureconfigMAX_SECURE_CONTEXTS         5

#endif /* FREERTOS_CONFIG_H */
