#ifndef __BSP_ADC_H
#define __BSP_ADC_H
#include "sys.h"
#include "ultrasion.h"
#include "touch.h"
#define ADC_Port1_GPIO_Port       GPIOF
#define ADC_Port2_GPIO_Port       GPIOA
#define ADC_Port3_GPIO_Port       GPIOC
#define ADC_Port4_GPIO_Port       GPIOB
#define ADC_Port5_GPIO_Port       GPIOF
#define ADC_Port6_GPIO_Port       GPIOA
#define ADC_Port7_GPIO_Port       GPIOC
#define ADC_Port8_GPIO_Port       GPIOB

#define ADC_Port1_Pin             GPIO_PIN_11
#define ADC_Port2_Pin             GPIO_PIN_6
#define ADC_Port3_Pin             GPIO_PIN_4
#define ADC_Port4_Pin             GPIO_PIN_1
#define ADC_Port5_Pin             GPIO_PIN_12
#define ADC_Port6_Pin             GPIO_PIN_7
#define ADC_Port7_Pin             GPIO_PIN_5
#define ADC_Port8_Pin             GPIO_PIN_0
#define ADC_BAT_Pin               GPIO_PIN_13
#define ADC_Single_Pin            GPIO_PIN_14

 
#define ADC_BAT_GPIO_Port         GPIOF
#define ADC_Single_GPIO_Port      GPIOF

#define  NONE_MIN 0.0f
#define  NONE_MAX 0.2f
#define  CULTRASONIC_DEV_MIN 2.0f
#define  CULTRASONIC_DEV_MAX 3.3f
#define  TOUCH_NONE_MAX_VALUE 0.80f
#define  TOUCH_NONE_MIN_Value 0.60f
#define  TOUCH_DOWN_MAX_VALUE 1.20f
#define  TOUCH_DOWN_MIN_VALUE 1.00f

// 添加必要的全局变量和结构体定义
#define FILTER_SIZE 2
#define FILTER_SIZE_PRO 16
 
typedef struct {
    float values[FILTER_SIZE];
	  float values_pro[FILTER_SIZE_PRO];
    uint8_t index;
    float sum;
} Filter_t;

typedef struct {
    float min_threshold;
    float max_threshold;
    float hysteresis;
    uint8_t stable_count;
    uint8_t last_type;
} DeviceDetector_t;

uint8_t DetectDeviceAdaptive(DeviceDetector_t* detector, float voltage);

float UpdateFilter(Filter_t* filter, float new_value);
float UpdataFilterPro(Filter_t* filter, float new_value);
void ADC_System_Init(void);
void ScanPortRevf(uint16_t *vref);
void ScanBat_VoicVREF(uint16_t *vref);
void bsp_InitADC(void);
 
#endif
