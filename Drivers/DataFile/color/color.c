#include "color.h"
#include "portagree.h"
#include "uart.h"
#include "devfile.h"
#include "network.h"
#include "network_data.h"
#include "stddef.h"

static ai_handle network;
static ai_u8 *activations = NULL;   // 改为指针
static DEV_COLOR __attribute__((section(".DMA_SRAM2_16KB"), aligned(4))) dev_color[8];
typedef struct __attribute__((packed)) {
	  uint32_t version;
	  float lux;
	  uint16_t ReadRaw,GreenRaw,BlueRaw;
}color_packet_t;
static HSV_COLOR hsv_color_list[]={

    /*Read color*/
	  {
		  .name = "RED",
          .number = 1,
			.h_filter_max = 6.0f,
			.h_filter_min = 5.0f,
			.s_filter_max = 0.90f,
			.s_filter_min = 0.70f,
			.v_filter_max = 256.0f,
			.v_filter_min = 230.0f,
		},

    /*Green color*/
	  {
		  .name = "GREEN",
      .number = 4,
			.h_filter_max = 130.0f,
			.h_filter_min = 120.0f,
			.s_filter_max = 0.40f,
			.s_filter_min = 0.30f,
			.v_filter_max = 180.0f,
			.v_filter_min = 150.0f,
		},

    /*Yellow color*/
	  {
		  .name = "YELLOW",
      .number = 2,
			.h_filter_max = 70.0f,
			.h_filter_min = 60.0f,
			.s_filter_max = 0.70f,
			.s_filter_min = 0.60f,
			.v_filter_max = 256.0f,
			.v_filter_min = 250.0f,
		},

    /*Blue color*/
	  {
		  .name = "BLUE",
      .number = 3,
			.h_filter_max = 230.0f,
			.h_filter_min = 210.0f,
			.s_filter_max = 0.80f,
			.s_filter_min = 0.60f,
			.v_filter_max = 256.0f,
			.v_filter_min = 200.0f,
		},

    /*DARK Read color*/
	  {
		  .name = "DARK_RED",
      .number = 6,
			.h_filter_max = 4.0f,
			.h_filter_min = 2.0f,
			.s_filter_max = 0.7f,
			.s_filter_min = 0.5f,
			.v_filter_max = 130.0f,
			.v_filter_min = 120.0f,
		},

    /*Black color*/
	  {
		  .name = "BLACK",
      .number = 7,
			.h_filter_max = 320.0f,
			.h_filter_min = 290.0f,
			.s_filter_max = 0.15f,
			.s_filter_min = 0.05f,
			.v_filter_max = 40.0f,
			.v_filter_min = 20.0f,
		},

    /*White color*/
	  {
		  .name = "WHITE",
      .number = 8,
			.h_filter_max = 0.0f,
			.h_filter_min = 0.0f,
			.s_filter_max = 0.0f,
			.s_filter_min = 0.0f,
			.v_filter_max = 256.0f,
			.v_filter_min = 200.0f,
		},

    /*Brown color*/
	  {
		  .name = "BROWN",
      .number = 5,
			.h_filter_max = 19.0f,
			.h_filter_min = 15.0f,
			.s_filter_max = 0.70f,
			.s_filter_min = 0.50f,
			.v_filter_max = 180.0f,
			.v_filter_min = 150.0f,
		},
};

// 滑动平均滤波函数
static float moving_average_filter(float input, float *buf, uint8_t size, uint8_t *index, uint8_t *count, float *sum) {
    if (*count < size) {
        // 窗口未满，直接加入
        buf[*count] = input;
        (*count)++;
        *sum += input;
        return *sum / (*count);
    } else {
        // 窗口已满，覆盖最旧数据
        *sum -= buf[*index];      // 减去要覆盖的值
        buf[*index] = input;      // 新值存入
        *sum += input;
        *index = (*index + 1) % size;
        return *sum / size;
    }
}

// 初始化函数（在设备初始化时调用）
void init_color_filter(DEV_COLOR *mt) {
    mt->flux_windex = 0;
    mt->flux_wcount = 0;
    mt->flux_sum = 0.0f;
    memset(mt->flux_window, 0, sizeof(mt->flux_window));
}

 static void rgbTohsv(int r,int g,int b,_HSV_T *hsv_t)
{
   float fr,fg,fb,max,min,delta = 0.0f;

	 fr = r;
	 fg = g;
	 fb = b;

	 max = max3(fr,fg,fb);
	 min = min3(fr,fg,fb);
	 delta = max - min;
	 if(delta == 0)
	 {
	   hsv_t->h = 0;
	 }
	 else
	 {
	   if(r == max)
			   hsv_t->h = ((g-b)/delta)*60.0f;
		 else if(g == max)
			   hsv_t->h = 120+(((b-r)/delta)*60.0f);
		 else if(b == max)
			   hsv_t->h = 240+(((r-g)/delta)*60.0f);
		 if(hsv_t->h<0)
			 hsv_t->h+=360.0f;
	 }
	 if(max == 0)
	    hsv_t->s = 0;
	 else
		  hsv_t->s = (float)(delta/max);

	 hsv_t->v = max;
	 hsv_t->color = (unsigned int)(((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff));
}

static bool isInRange(float h, float s, float v, const HSV_COLOR *color) {
    return (h >= color->h_filter_min && h <= color->h_filter_max &&
            s >= color->s_filter_min && s <= color->s_filter_max &&
            v >= color->v_filter_min && v <= color->v_filter_max);
} 

const HSV_COLOR* matchingcolor(float h,float s,float v)
{
	 const HSV_COLOR *closest = NULL;
	 float smallest_difference = FLT_MAX;

		for (int i = 0; i <
		(sizeof(hsv_color_list) / sizeof(hsv_color_list[0]))
		; i++) {
			if (isInRange(h, s, v, &hsv_color_list[i])) {
				return &hsv_color_list[i];
			} else {
				float dh = fabs(h - (hsv_color_list[i].h_filter_max + hsv_color_list[i].h_filter_min) / 2.0f);
				if (dh > (hsv_color_list[i].h_filter_max - hsv_color_list[i].h_filter_min) / 2.0f) {
					dh = fabs(h - hsv_color_list[i].h_filter_max) < fabs(h - hsv_color_list[i].h_filter_min) ?
						 fabs(h - hsv_color_list[i].h_filter_max) : fabs(h - hsv_color_list[i].h_filter_min);
				}
				float ds = fabs(s - (hsv_color_list[i].s_filter_max + hsv_color_list[i].s_filter_min) / 2.0f);
				float dv = fabs(v - (hsv_color_list[i].v_filter_max + hsv_color_list[i].v_filter_min) / 2.0f);
				float difference = dh + ds + dv;
				if (difference < smallest_difference) {
					smallest_difference = difference;
					closest = &hsv_color_list[i];
				}
			}
		}

		return closest; // 如果没有完全匹配，则返回最接近的颜色

}

 void *pvPortMallocAligned(size_t size, size_t align)
{
    void *p = pvPortMalloc(size + align - 1);
    if (p != NULL) {
        uintptr_t addr = (uintptr_t)p;
        addr = (addr + align - 1) & ~(align - 1);
        return (void*)addr;
    }
    return NULL;
}
void colorAi_init(void)
{
    ai_error err;
	
		const size_t activations_size = AI_NETWORK_DATA_ACTIVATIONS_SIZE;
    activations = pvPortMallocAligned(activations_size,32);
    if (activations == NULL) {
        // 分配失败处理
        USB_printf("Failed to allocate activations buffer\r\n");
        while(1);
    }    
    // 创建激活缓冲区地址数组（必须是指针数组）
    const ai_handle act_addr[] = { activations };
    
    // 创建并初始化模型实例
    // 第三个参数传 NULL 表示权重已内嵌在代码中（常规方式）[citation:10]
    err = ai_network_create_and_init(&network, act_addr, NULL);
    
    if (err.type != AI_ERROR_NONE) {
        // 初始化失败处理
        printf("AI init failed: type=%d, code=%d\r\n", err.type, err.code);
        while(1);
    }
    
    // 获取输入/输出缓冲区（可选）
    ai_buffer *ai_input = ai_network_inputs_get(network, NULL);
    ai_buffer *ai_output = ai_network_outputs_get(network, NULL);
}

// RGB 转 HSV 函数
static void rgb_to_hsv(uint16_t r, uint16_t g, uint16_t b, float *h, float *s, float *v)
{
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    
    float max = rf, min = rf;
    if (gf > max) max = gf; if (gf < min) min = gf;
    if (bf > max) max = bf; if (bf < min) min = bf;
    float delta = max - min;
    
    // Hue
    if (delta == 0) *h = 0;
    else if (max == rf) *h = 60 * fmodf((gf - bf) / delta, 6);
    else if (max == gf) *h = 60 * ((bf - rf) / delta + 2);
    else *h = 60 * ((rf - gf) / delta + 4);
    if (*h < 0) *h += 360;
    *h /= 360.0f;
    
    // Saturation
    *s = (max == 0) ? 0 : delta / max;
    
    // Value
    *v = max;
}
uint8_t color_predict(uint16_t r, uint16_t g, uint16_t b)
{
    float h, s, v;
    rgb_to_hsv(r, g, b, &h, &s, &v);
    
    // 准备输入（根据量化参数调整，假设输入为 int8）
    int8_t input_data[3];
    input_data[0] = (int8_t)(h * 255 - 128);
    input_data[1] = (int8_t)(s * 255 - 128);
    input_data[2] = (int8_t)(v * 255 - 128);
    
    int8_t output_data[2];
    
    // 获取输入输出缓冲区
    ai_buffer *ai_input = ai_network_inputs_get(network, NULL);
    ai_buffer *ai_output = ai_network_outputs_get(network, NULL);
    
    ai_input[0].data = input_data;
    ai_output[0].data = output_data;
    
    // 运行推理
    ai_network_run(network, ai_input, ai_output);
    
    // 返回概率最高的类别
    return (output_data[0] > output_data[1]) ? 0 : 1;
}

/**
 * @brief 将 RGB 和标签写入 CSV 文件
 * @param r 红色值 (0-255)
 * @param g 绿色值 (0-255)
 * @param b 蓝色值 (0-255)
 * @param label 颜色类别 (0,1,2...)
 */
void log_color_data(uint16_t r, uint16_t g, uint16_t b, uint8_t label)
{
    char line[32];
    UINT bw;
    int len;
    _IO_FILE *file_handle = getFatfsHandle();
    len = snprintf(line, sizeof(line), "%u,%u,%u,%u\r\n", r, g, b, label);
    if (len > 0) {
        f_write(&file_handle->ioFile, line, len, &bw);
        /* 可选：立即同步到物理介质（确保数据不丢失，但会降低性能） */
        f_sync(&file_handle->ioFile);
    }
}

void data_create_csv(void)
{ 
   _IO_FILE *file_handle = getFatfsHandle();
 
	 f_open(&file_handle->ioFile,"1:color_rgb.csv",FA_CREATE_ALWAYS|FA_WRITE);
}
/* 采集任务 */
void data_collection(uint8_t r,uint8_t g,uint8_t b,uint8_t current_lab)
{
		log_color_data(r,g,b,current_lab);

		/* 打印提示，便于观察 */
//		USB_printf("Logged: R=%d G=%d B=%d label=%d\r\n",r,g, b, current_lab);
    vTaskDelay(10);
}
void close_collection(void)
{ 
	 _IO_FILE *file_handle = getFatfsHandle();
   f_close(&file_handle->ioFile);
}
DEV_COLOR *read_color(void *self)
{ 
	 DEV_COLOR *mt = (DEV_COLOR*)self;
	 return mt;   
}
void refsh_color(DEV_COLOR *mt, uint8_t* data)
{ 
		color_packet_t pkt;
    memcpy(&pkt, data, sizeof(color_packet_t)); 
    float fr,fg,fb;
    sscanf((const char*)data,"%d/%d/%d/%d/%d",&mt->rawR,
																							&mt->rawG,
																							&mt->rawB,
																							&mt->rawLux,
																						  &mt->version);
	  fr = (float)(mt->rawR/R_WHITE);
    fg = (float)(mt->rawG/G_WHITE);
	  fb = (float)(mt->rawB/B_WHITE);
 

	  mt->RGBL[0] = fr >255?255:fr;
	  mt->RGBL[1] = fg >255?255:fg;
	  mt->RGBL[2] = fb >255?255:fb;
		
 
    // uint8_t color = color_predict(mt->RGBL[0], mt->RGBL[1], mt->RGBL[2]);
   // USB_printf("color: %s\r\n", color == 0 ? "Red" : "Green");
    
		float flux = (float)mt->rawLux >20000.0f?20000.0f:(float)mt->rawLux;

    // 滑动平均滤波
    float filtered_flux = moving_average_filter(flux, 
                                                mt->flux_window, 
                                                WINDOW_SIZE,
                                                &mt->flux_windex,
                                                &mt->flux_wcount,
                                                &mt->flux_sum);		
	  rgbTohsv(mt->RGBL[0],mt->RGBL[1],mt->RGBL[2],&mt->hsv); 
		mt->RGBL[3] = filtered_flux * 999.0f / 20000.0f;
 
	  mt->is_resh_flag = true;
}

DEV_COLOR *create_color(uint8_t index)
{ 
 	 DEV_COLOR *color = &dev_color[index];
	
	 memset(color,0,sizeof(DEV_COLOR));
	 color->base.type = DEV_ID_COLOR;
	 memset(color->base.name,0,sizeof(color->base.name));
	 strcpy(color->base.name,"color"); 
	 
	 color->sensor_params.black_ratio = 0.7f;
	 color->sensor_params.white_ratio = 1.3f;
	 color->sensor_params.hysteresis_ratio = 0.05f;
	 color->sensor_params.min_samples = 100;
	 
	 init_color_filter(color);
	 color->is_key_value = false;
	
	 return color;	   
}

void free_color(uint8_t port)
{ 
   DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));	
   if(color!=NULL)
   {
	     memset(color,0,sizeof(DEV_COLOR));	
	 }	  
}
