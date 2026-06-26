#ifndef __API_KEY_H
#define __API_KEY_H
#include "sys.h"

#define Enable_Key_Clock  __HAL_RCC_GPIOD_CLK_ENABLE

#define Key_Right_Group   GPIOD
#define Key_Right_Pin     GPIO_PIN_2

#define Key_Left_Group    GPIOD
#define Key_Left_Pin      GPIO_PIN_3

#define Key_Up_Group      GPIOD
#define Key_Up_Pin        GPIO_PIN_4

#define key_3_val 			HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_4)
#define key_4_val 			HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3)
#define key_5_val 			HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_5)

#define KEY_FILTER_TIME		3
#define KEY_LONG_TIME		0

#define KEY_DOWN            1
#define KEY_UP              0

typedef struct
{

	uint8_t 	(*IsKeyDownFunc)(void);
	uint8_t 	Count;
	uint8_t 	FilterTime;
	uint16_t	LongCount;
	uint16_t 	LongTime;
	uint8_t 	State;
	uint8_t		KeyCodeUp;
	uint8_t		KeyCodeDown;
	uint8_t		KeyCodeLong;
	uint8_t 	RepeatSpeed;
	uint8_t 	RepeatCount;
}KEY_T;


typedef enum
{
	KEY_NONE = 0,

	KEY_3_DOWN,
	KEY_3_UP,
	KEY_3_LONG,

	KEY_4_DOWN,
	KEY_4_UP,
	KEY_4_LONG,

	KEY_5_DOWN,
	KEY_5_UP,
	KEY_5_LONG,

	KEY_6_DOWN,
	KEY_6_UP,
	KEY_6_LONG,

	KEY_34_DOWN,
	KEY_34_UP,
	KEY_34_LONG,
}KEY_ENUM;


typedef enum
{
	KID_K3 = 0,
	KID_K4,
	KID_K5,
	KID_K6,
}KEY_ID_E;

/*遥控器*/
enum{
   /*方向值*/
   KeyUp = 0,
   KeyDown,
   KeyRight,
   KeyLeft,
   /*功能值*/
   yKey,
   aKey,
   bKey,
   xKey,
   /*L1,L2,R1,R2*/
   R1Key,
   R2Key,
   L2Key,
   L1Key,
   /*rocker*/
   L_Rocker_X,
   L_Rocker_Y,
   R_Rocker_X,
   R_Rocker_Y,
};

typedef struct
{ 
   uint8_t keyLeft,keyMiddle,keyRight;
	 uint8_t *keyMusic,*LinkeMusic;
	 uint32_t keyMusicSize,LinkeMusicSize;
}BSP_KEY;

#define KEY_FIFO_SIZE	10

typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];
	uint8_t Read;
	uint8_t Write;
}KEY_FIFO_T;



uint8_t bsp_GetKey(void);
uint8_t bsp_GetKeyState(uint8_t _ucKeyID);
void bsp_InitKey(void);
void bsp_KeyScan(void);
void bsp_PutKey(uint8_t _KeyCode);
void bsp_ClearKey(void);

void updata_key_value(void);
BSP_KEY *getKeyHandle(void);

bool getShutdownPending(void);
uint8_t getKeyEntery(void);
#endif
