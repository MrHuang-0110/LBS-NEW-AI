#ifndef __BSP_IIC_H
#define __BSP_IIC_H

#include "sys.h"
typedef unsigned char u8;
/*0--Software
  1--HardWare*/
#define IIC_SALVE_MODE    1

#define I2Cx_FLAG_TIMEOUT             ((uint32_t) 1000) //0x1100
#define I2Cx_LONG_TIMEOUT             ((uint32_t) (300 * I2Cx_FLAG_TIMEOUT)) //was300

/*引脚定义*/

#define SENSORS_I2C_SCL_GPIO_PORT         		GPIOF
#define SENSORS_I2C_SCL_GPIO_CLK_ENABLE()    	__GPIOF_CLK_ENABLE()
#define SENSORS_I2C_SCL_GPIO_PIN         		  GPIO_PIN_1

#define SENSORS_I2C_SDA_GPIO_PORT         		GPIOF
#define SENSORS_I2C_SDA_GPIO_CLK_ENABLE()   	__GPIOF_CLK_ENABLE()
#define SENSORS_I2C_SDA_GPIO_PIN          		GPIO_PIN_0

#define SENSORS_I2C_AF                    		GPIO_AF6_I2C5

#define SENSORS_I2C              		  		I2C5
#define SENSORS_I2C_RCC_CLK_ENABLE()     		__HAL_RCC_I2C5_CLK_ENABLE()

#define SENSORS_I2C_FORCE_RESET()      			__HAL_RCC_I2C5_FORCE_RESET()
#define SENSORS_I2C_RELEASE_RESET()    			__HAL_RCC_I2C5_RELEASE_RESET()

/*信息输出*/
#define I2C_DEBUG_ON         0
#define I2C_DEBUG_FUNC_ON    0

#define I2C_INFO(fmt,arg...)           USB_printf("<<-I2C-INFO->> "fmt"\n",##arg)
#define I2C_ERROR(fmt,arg...)          USB_printf("<<-I2C-ERROR->> "fmt"\n",##arg)
#define I2C_DEBUG(fmt,arg...)          do{\
                                          if(I2C_DEBUG_ON)\
                                          USB_printf("<<-I2C-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)

#define I2C_DEBUG_FUNC()               do{\
                                         if(I2C_DEBUG_FUNC_ON)\
                                         USB_printf("<<-I2C-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)


#define I2C_BUFFER_SIZE 512

#if 1
//IO方向设置
#define SDA_IN()  {GPIOF->MODER&=~(3<<(0));GPIOF->MODER|=0<<(0);}
#define SDA_OUT() {GPIOF->MODER&=~(3<<(0));GPIOF->MODER|=1<<(0);}
#endif

/* 引脚 定义 */
#define IIC_SCL_GPIO_PORT               GPIOF
#define IIC_SCL_GPIO_PIN                GPIO_PIN_1
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)   /* PH口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOF
#define IIC_SDA_GPIO_PIN                GPIO_PIN_0
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)   /* PH口时钟使能 */

/* IO操作 */
#define IIC_SCL(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SCL */

#define IIC_SDA(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SDA */

#define IIC_READ_SDA     HAL_GPIO_ReadPin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN) /* 读取SDA */


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void I2cMaster_Init(void);
int Sensors_I2C_WriteRegister(unsigned char slave_addr,
                                        unsigned char reg_addr,
                                        unsigned short len,
                                        unsigned char *data_ptr);
int Sensors_I2C_ReadRegister(unsigned char slave_addr,
                                       unsigned char reg_addr,
                                       unsigned short len,
                                       unsigned char *data_ptr);


#endif



