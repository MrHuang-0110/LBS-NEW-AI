#include "_camer.h"
#include "camer.h"
#include "uart.h"
#include "portagree.h"

 
void _camer_changer_camer_mode(PikaObj *self, pika_float port, pika_float mode)
{
  DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
  if(camer== NULL)return;
 
  uint8_t u8_data = (uint8_t)mode;
	
		MultiUart_SendFrame((UartDevice_t)port,
												&u8_data,
												1,
												DEV_ID_CAMER,
												0xFF,
												10,250);
 
}

pika_float _camer_cam_data(PikaObj *self, pika_float port, pika_float id, pika_float obj_id)
{
    // 1. 获取摄像头设备
    DEV_CAMER *camer = read_camer((SensorBase *)getDevBase(port));
    if (camer == NULL) return 0.0f;

    // 2. 参数范围检查
    int group = (int)id;
    int field = (int)obj_id;
    if (group < 0 || group >= 4 || field < 0 || field >= 6)
        return 0.0f;

    // 3. 获取数组指针
    uint8_t *data = camer->data;   // 假设为 uint8_t 数组
    int offset = group * 10;                   // 每组10字节

    // 4. 根据 field 提取数据
    switch (field) {
        case 0: // ID标记
            return (pika_float)data[offset];
				case 1: // x坐标，2字节大端
						return (pika_float)((data[offset + 1] << 8) | data[offset + 2]);
				case 2: // y坐标，2字节大端
						return (pika_float)((data[offset + 3] << 8) | data[offset + 4]);
				case 3: // 宽度，2字节大端
						return (pika_float)((data[offset + 5] << 8) | data[offset + 6]);
				case 4: // 高度，2字节大端
						return (pika_float)((data[offset + 7] << 8) | data[offset + 8]);
        case 5: // 置信度
            return (pika_float)data[offset + 9];
    }
		return 0;
}
