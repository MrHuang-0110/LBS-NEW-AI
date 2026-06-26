#ifndef __VALUETYPE_H
#define __VALUETYPE_H
#include "sys.h"
#include <stdint.h>
#include <stddef.h>

// 数据类型定义
typedef enum {
    DATA_TYPE_INT8 = 0x01,
    DATA_TYPE_UINT8 = 0x02,
    DATA_TYPE_INT16 = 0x03,
    DATA_TYPE_UINT16 = 0x04,
    DATA_TYPE_INT32 = 0x05,
    DATA_TYPE_UINT32 = 0x06,
    DATA_TYPE_FLOAT = 0x07,
    DATA_TYPE_DOUBLE = 0x08,
    DATA_TYPE_STRING = 0x09,
    DATA_TYPE_CUSTOM = 0x0A,
    DATA_TYPE_PID_PARAMS = 0x0B  // 自定义PID参数类型
} DataType;
 
#endif
