#include "iic.h"
#include "delay.h"

static I2C_HandleTypeDef I2C_Handle;

uint8_t g_i2cTxBuf[I2C_BUFFER_SIZE]; /* 发送缓冲 */
uint8_t g_i2cRxBuf[I2C_BUFFER_SIZE]; /* 接收缓冲 */

uint32_t g_i2cLen;

 
static void I2Cx_Error(uint8_t Addr)
{
	/* 恢复I2C寄存器为默认值 */
	HAL_I2C_DeInit(&I2C_Handle);
	/* 重新初始化I2C外设 */
	I2cMaster_Init();
}
 
void I2cMaster_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

		/* 使能I2Cx时钟 */
		SENSORS_I2C_RCC_CLK_ENABLE();

		/* 使能I2C GPIO 时钟 */
		SENSORS_I2C_SCL_GPIO_CLK_ENABLE();
		SENSORS_I2C_SDA_GPIO_CLK_ENABLE();

		/* 配置I2Cx引脚: SCL ----------------------------------------*/
		GPIO_InitStructure.Pin =  SENSORS_I2C_SCL_GPIO_PIN;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStructure.Pull= GPIO_NOPULL;
		GPIO_InitStructure.Alternate=SENSORS_I2C_AF;
		HAL_GPIO_Init(SENSORS_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

		/* 配置I2Cx引脚: SDA ----------------------------------------*/
		GPIO_InitStructure.Pin = SENSORS_I2C_SDA_GPIO_PIN;
		HAL_GPIO_Init(SENSORS_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

		if(HAL_I2C_GetState(&I2C_Handle) == HAL_I2C_STATE_RESET)
		{
			/* 强制复位I2C外设时钟 */
			SENSORS_I2C_FORCE_RESET();

			/* 释放I2C外设时钟复位 */
			SENSORS_I2C_RELEASE_RESET();

			/* I2C 配置 */
			I2C_Handle.Instance = SENSORS_I2C;
			I2C_Handle.Init.Timing			 = 0x40604E73;//100KHz
			I2C_Handle.Init.OwnAddress1 	 = 0;
			I2C_Handle.Init.AddressingMode	 = I2C_ADDRESSINGMODE_7BIT;
			I2C_Handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
			I2C_Handle.Init.OwnAddress2 	 = 0;
			I2C_Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			I2C_Handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
			I2C_Handle.Init.NoStretchMode	 = I2C_NOSTRETCH_DISABLE;

			/* 初始化I2C */
			HAL_I2C_Init(&I2C_Handle);
			/* 使能模拟滤波器 */
			HAL_I2CEx_AnalogFilter_Config(&I2C_Handle, I2C_ANALOGFILTER_ENABLE);

		}


}
void IIC_Init(void)
{
	 #if IIC_SALVE_MODE
	   I2cMaster_Init();
	 #else
    IIC_SDA(1);
    IIC_SCL(1);
	 #endif
}

 #if IIC_SALVE_MODE == 0
 
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA(1);
	IIC_SCL(1);
	delay_us(9);
 	IIC_SDA(0);	 //START:when CLK is high,DATA change form high to low
	delay_us(9);
	IIC_SCL(0);  //钳住I2C总线，准备发送或接收数据
}
 
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL(0);
	delay_us(9);
	IIC_SDA(0);	//STOP:when CLK is high DATA change form low to high
 	delay_us(9);
	IIC_SCL(1);
	delay_us(9);
	IIC_SDA(1);//发送I2C总线结束信号
	delay_us(9);
}
 
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
    SDA_IN();      //SDA设置为输入
	IIC_SDA(1);delay_us(7);
	IIC_SCL(1);delay_us(7);
	while(IIC_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0);//时钟输出0
	return 0;
}
 
void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(21);
	IIC_SCL(1);
	delay_us(21);
	IIC_SCL(0);
}
//不产生ACK应答
void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(21);
	IIC_SCL(1);
	delay_us(21);
	IIC_SCL(0);
}
 
void IIC_Send_Byte(u8 txd)
{
    u8 t;
	SDA_OUT();
    IIC_SCL(0);//拉低时钟开始数据传输
	    for(t = 0;t<8;t++)
	    {
			   if(txd&0x80)
				 {
				   IIC_SDA(1);
				 }
				 else
				 {
				   IIC_SDA(0);
				 }
				 txd<<=1;
				 delay_us(11);
				 IIC_SCL(1);
				 delay_us(9);
				 IIC_SCL(0);
         delay_us(9);
			}
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
	for(i=0;i<8;i++ )
	{
		IIC_SCL(0);
		delay_us(5);
		IIC_SCL(1);
		receive<<=1;
	   if(IIC_READ_SDA)
		 {
		    receive |=0x01;
		 }
		 delay_us(11);
	}
	if(!ack)
	{
	   IIC_NAck();//发送nACK
	}
	else
	{
	  IIC_Ack(); //发送ACK
	}
	return receive;
}
static void dev_iic_write(uint8_t addr,uint8_t reg,uint8_t data)
{
     IIC_Start();

	 IIC_Send_Byte(addr);

	 IIC_Ack();

	 IIC_Send_Byte(reg);

	 IIC_Ack();

	 IIC_Send_Byte(data);

	 IIC_Ack();

	 IIC_Stop();
}

static uint8_t dev_iic_read(uint8_t ReadAddr,uint8_t reg)
{
	 char data;

     IIC_Start();

	 IIC_Send_Byte(ReadAddr - 1);

	 IIC_Ack();

	 IIC_Send_Byte(reg);

	 IIC_Ack();

	 IIC_Start();

	 IIC_Send_Byte(ReadAddr);

	 IIC_Ack();

	 data = IIC_Read_Byte(0);

	 IIC_Stop();

	 return data;
}
 
static int32_t iic_write_reg_more(uint16_t addr,uint16_t reg,uint8_t *data,uint16_t length)
{
    uint8_t loop;

    for(loop = 0;loop<length;loop++)
    {
         dev_iic_write(addr - 1,reg+loop,data[loop]);
    }

    return 0;
}

static int32_t iic_read_reg_more(uint16_t addr,uint16_t reg,uint8_t *data,uint16_t length)
{
    uint8_t loop;

    for(loop = 0;loop<length;loop++)
    {
         data[loop] = dev_iic_read(addr,reg+loop);
    }

     return 0;
}
#endif
int Sensors_I2C_WriteRegister(unsigned char slave_addr,
                                        unsigned char reg_addr,
                                        unsigned short len,
                                        unsigned char *data_ptr)
{


    #if IIC_SALVE_MODE
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_I2C_Mem_Write(&I2C_Handle, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT,data_ptr, len,I2Cx_FLAG_TIMEOUT);
	 if(status != HAL_OK)
	 {
	 	I2Cx_Error(slave_addr);
 	}
	 while (HAL_I2C_GetState(&I2C_Handle) != HAL_I2C_STATE_READY);
	 while (HAL_I2C_IsDeviceReady(&I2C_Handle, slave_addr, I2Cx_FLAG_TIMEOUT, I2Cx_FLAG_TIMEOUT) == HAL_TIMEOUT);
	 while (HAL_I2C_GetState(&I2C_Handle) != HAL_I2C_STATE_READY);
	 return status;
	#else
    iic_write_reg_more(slave_addr,reg_addr,data_ptr,len);
	return 0;
	#endif

}


int Sensors_I2C_ReadRegister(unsigned char slave_addr,
                                       unsigned char reg_addr,
                                       unsigned short len,
                                       unsigned char *data_ptr)
{
   #if IIC_SALVE_MODE
	HAL_StatusTypeDef status = HAL_OK;
	status =HAL_I2C_Mem_Read(&I2C_Handle,slave_addr,reg_addr,I2C_MEMADD_SIZE_8BIT,data_ptr,len,I2Cx_FLAG_TIMEOUT);
	 if(status != HAL_OK)
	 {
		 I2Cx_Error(slave_addr);
	 }
 	  while (HAL_I2C_GetState(&I2C_Handle) != HAL_I2C_STATE_READY);
 	  while (HAL_I2C_IsDeviceReady(&I2C_Handle, slave_addr, I2Cx_FLAG_TIMEOUT, I2Cx_FLAG_TIMEOUT) == HAL_TIMEOUT);
  	while (HAL_I2C_GetState(&I2C_Handle) != HAL_I2C_STATE_READY);
	  return status;
   #else
   iic_read_reg_more(slave_addr,reg_addr,data_ptr,len);
   return 0;
   #endif

}


