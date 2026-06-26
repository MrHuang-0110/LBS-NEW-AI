#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#define I2C_SDA_IO_GPIO_Port      GPIOF
#define I2C_SCL_IO_GPIO_Port      GPIOF
#define SoundPower_IO_GPIO_Port   GPIOB
#define BlueState_IO_GPIO_Port    GPIOD
#define SystemLED_IO_GPIO_Port    GPIOG
#define SystemPower_IO_GPIO_Port  GPIOD
#define MiddKey_IO_GPIO_Port      GPIOD
#define LeftKey_IO_GPIO_Port      GPIOD
#define RightKey_IO_GPIO_Port     GPIOD
#define BlueStateLED_IO_GPIO_Port GPIOD

#define I2C_SDA_IO_Pin          GPIO_PIN_0
#define I2C_SCL_IO_Pin          GPIO_PIN_1
#define SoundPower_IO_Pin       GPIO_PIN_2
#define BlueState_IO_Pin        GPIO_PIN_10
#define SystemLED_IO_Pin        GPIO_PIN_7
#define SystemPower_IO_Pin      GPIO_PIN_2
#define MiddKey_IO_Pin          GPIO_PIN_3
#define LeftKey_IO_Pin          GPIO_PIN_4
#define RightKey_IO_Pin         GPIO_PIN_5
#define BlueStateLED_IO_Pin     GPIO_PIN_6

void bsp_InitGpio(void);

#endif

