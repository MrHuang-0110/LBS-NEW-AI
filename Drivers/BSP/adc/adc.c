#include "adc.h"

static ADC_HandleTypeDef hadc1;
static ADC_HandleTypeDef hadc2;
static uint32_t HAL_RCC_ADC12_CLK_ENABLED=0;
 
// 全局变量
Filter_t adc_filters[8];
Filter_t voic_bat_filters[2];

DeviceDetector_t device_detectors[8];
float vdda_voltage = 3.3f; // 将在初始化时更新

static void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 8;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    while(1){;}
  }


  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
   while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }


  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(1){;}
  }
}

static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
  hadc2.Init.Resolution = ADC_RESOLUTION_16B;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 2;
  hadc2.Init.DiscontinuousConvMode = ENABLE;
  hadc2.Init.NbrOfDiscConversion = 1;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    while(1){;}
  }

  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    while(1){;}
  }

  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    while(1){;}
  }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = ADC_Port2_Pin|ADC_Port6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_Port3_Pin|ADC_Port7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_Port8_Pin|ADC_Port4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_Port1_Pin|ADC_Port5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }
  else if(hadc->Instance==ADC2)
  {

    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = ADC_BAT_Pin|ADC_Single_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }

}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }
    HAL_GPIO_DeInit(GPIOA, ADC_Port2_Pin|ADC_Port6_Pin);
    HAL_GPIO_DeInit(GPIOC, ADC_Port3_Pin|ADC_Port7_Pin);
    HAL_GPIO_DeInit(GPIOB, ADC_Port8_Pin|ADC_Port4_Pin);
    HAL_GPIO_DeInit(GPIOF, ADC_Port1_Pin|ADC_Port5_Pin);
  }
  else if(hadc->Instance==ADC2)
  {
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }
    HAL_GPIO_DeInit(GPIOF, ADC_BAT_Pin|ADC_Single_Pin);
  }
}
 

// 初始化函数（在系统启动时调用一次）
void ADC_System_Init(void)
{
	  MX_ADC1_Init();
    MX_ADC2_Init();
 
    // 执行ADC校准
   // HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
    
    // 初始化滤波器和设备检测器
    for(int i = 0; i < 8; i++) {
        // 初始化滤波器
        for(int j = 0; j < FILTER_SIZE; j++) {
            adc_filters[i].values[j] = 0.0f;
        }
        adc_filters[i].index = 0;
        adc_filters[i].sum = 0.0f;
        
        // 初始化设备检测器
        device_detectors[i].min_threshold = NONE_MIN;
        device_detectors[i].max_threshold = NONE_MAX;
        device_detectors[i].hysteresis = 0.05f;
        device_detectors[i].stable_count = 0;
        device_detectors[i].last_type = 0;
    }
    
		for(int i = 0;i<2;i++)
	  { 
        for(int j = 0; j < FILTER_SIZE_PRO; j++) {
            voic_bat_filters[i].values_pro[j] = 0.0f;
        }		    
        voic_bat_filters[i].index = 0;
        voic_bat_filters[i].sum = 0.0f;				
		}
    // 测量实际VDDA电压
   // MeasureVDDA();
		
}

// 更新滤波器函数
float UpdateFilter(Filter_t* filter, float new_value)
{
    // 移除最旧的值
    filter->sum -= filter->values[filter->index];
    
    // 添加新值
    filter->values[filter->index] = new_value;
    filter->sum += new_value;
    
    // 更新索引
    filter->index = (filter->index + 1) % FILTER_SIZE;
    
    // 返回平均值
    return filter->sum / FILTER_SIZE;
}
float UpdataFilterPro(Filter_t* filter, float new_value)
{ 
    // 移除最旧的值
    filter->sum -= filter->values_pro[filter->index];
    
    // 添加新值
    filter->values_pro[filter->index] = new_value;
    filter->sum += new_value;
    
    // 更新索引
    filter->index = (filter->index + 1) % FILTER_SIZE_PRO;
    
    // 返回平均值
    return filter->sum / FILTER_SIZE_PRO;    
}
// 设备识别函数（使用自适应阈值）
uint8_t DetectDeviceAdaptive(DeviceDetector_t* detector, float voltage)
{
    uint8_t current_type = 0;
    
    // 根据电压确定设备类型
    if(voltage >= NONE_MIN && voltage <= NONE_MAX) {
        current_type = 0;
    } else if(voltage >= CULTRASONIC_DEV_MIN && voltage <= CULTRASONIC_DEV_MAX) {
        current_type = DEV_ID_ULTRASION;
    } else if(voltage >= TOUCH_NONE_MIN_Value && voltage <= TOUCH_NONE_MAX_VALUE) {
        current_type = DEV_ID_TOUCH;
    }
    
    // 使用滞回防止抖动
    if(current_type != 0) {
        detector->stable_count++;
    } else {
        detector->stable_count = 0;
			  detector->last_type = 0;
    }
    
    // 只有连续多次检测到相同类型才确认
    if(detector->stable_count >= 3) {
        detector->last_type = current_type;
        
        // 根据稳定读数微调阈值
        if(detector->stable_count > 10) {
            detector->min_threshold = (detector->min_threshold * 0.9f) + (voltage * 0.1f);
            detector->max_threshold = (detector->max_threshold * 0.9f) + (voltage * 0.1f);
        }
    }
    
    return detector->last_type;
}

 void ScanPortRevf(uint16_t *vref)
{ 

	 for(uint8_t j = 0; j < 8; j++) // 采样16次
	{         
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 10); 
      vref[j]= HAL_ADC_GetValue(&hadc1);		   
	}
		HAL_ADC_Stop(&hadc1);	    
 
    // 数据交换（如果仍然需要）
    uint16_t tempVref = vref[0];
    vref[0] = vref[3];
    vref[3] = tempVref;

    tempVref = vref[1];
    vref[1] = vref[2];
    vref[2] = tempVref;
}

void ScanBat_VoicVREF(uint16_t *vref)
{
	 for(uint8_t i = 0;i<2;i++)
	 { 
			HAL_ADC_Start(&hadc2);
			HAL_ADC_PollForConversion(&hadc2,10);	
      vref[i] =  HAL_ADC_GetValue(&hadc2); 
	 } 
	  HAL_ADC_Stop(&hadc2);  
}

void bsp_InitADC(void)
{   
	  ADC_System_Init();
}
