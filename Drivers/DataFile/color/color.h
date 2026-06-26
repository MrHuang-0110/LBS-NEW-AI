#ifndef __COLOR_H
#define __COLOR_H
#include "sys.h"
#define DEV_ID_COLOR          0xA2

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max3(a,b,c) (((a) > (b) ? (a) : (b)) > (c) ? ((a) > (b) ? (a) : (b)) : (c))
#define min3(a,b,c) (((a) < (b) ? (a) : (b)) < (c) ? ((a) < (b) ? (a) : (b)) : (c))
#define R_WHITE   8.502196078431373f
#define G_WHITE   19.77647058823529f
#define B_WHITE   15.48917647058824f
#define LUX_WHITE 19.77647058823529f

#define ADC_RESOLUTION 4095.0f  // 12位ADC
#define BUFFER_SIZE 16          // 滑动窗口大小
#define HYSTERESIS_RATIO 0.05f  // 迟滞比例
#define MIN_THRESHOLD_RANGE 50.0f // 最小阈值范围，防止在低光环境下阈值过小
#define ADAPTATION_RATE 0.1f    // 自适应速率
#define WINDOW_SIZE 8

typedef enum {
    BACKGROUND = 0,
	  BLACK_LINE = 1,
	  WHITE_LINE = 0,
} LineType;

typedef struct {
    uint16_t raw_value;
    float filtered;
    float baseline;
    float black_threshold;
    float white_threshold;
    LineType current_line;
    LineType last_confirmed;
    uint8_t confirm_count;
    float history[BUFFER_SIZE];
    uint8_t index;
    float min_observed;  // 观察到的最小值（用于动态范围调整）
    float max_observed;  // 观察到的最大值（用于动态范围调整）
    uint32_t sample_count; // 采样计数（用于统计）
} SensorData;

// 全局变量，存储传感器的特性参数
typedef struct {
    float black_ratio;      // 黑线相对于基线的比例
    float white_ratio;      // 白线相对于基线的比例
    float hysteresis_ratio; // 迟滞比例
    uint16_t min_samples;   // 最小采样数 before 开始自适应
} SensorParams;

typedef struct
{
  float h;
  float s;
  float v;
  int color;
}_HSV_T;

typedef struct
{
   char *name;
   unsigned char number;

   float h_filter_max;
   float h_filter_min;

   float s_filter_max;
   float s_filter_min;

   float v_filter_max;
   float v_filter_min;
}HSV_COLOR;

typedef struct {
  uint8_t Training_Data_RGB[3];
	uint8_t lable;
}COLOR_AI_TRAINING;

typedef struct  
{
	 SensorBase base;
	 SensorParams sensor_params;
	 SensorData sensor_data;
	  
   int rawR,rawG,rawB,rawLux;
   int RGBL[5];
		float flux_window[WINDOW_SIZE];  // 环形缓冲区
    uint8_t flux_windex;             // 当前写入位置
    uint8_t flux_wcount;             // 已存入的有效数据个数
    float flux_sum;                  // 窗口内值的和（避免重复求和）
   int version;
	 uint8_t lable,color_start,color_end;
	 bool is_resh_flag;
	 bool is_key_value;
   _HSV_T hsv;
}DEV_COLOR;

 
 
const HSV_COLOR* matchingcolor(float h,float s,float v);

void colorAi_init(void);
uint8_t color_predict(uint16_t r, uint16_t g, uint16_t b);
void data_create_csv(void);
void data_collection(uint8_t r,uint8_t g,uint8_t b,uint8_t current_lab);
void close_collection(void);
DEV_COLOR *read_color(void *self);
void refsh_color(DEV_COLOR *mt, uint8_t* data);
DEV_COLOR *create_color(uint8_t index);
void free_color(uint8_t port);
 
#endif
