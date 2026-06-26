#include "key.h"
#include "devfile.h"
#include "matchineState.h"
static BSP_KEY key;
#if 0
static uint8_t GetRemoteControlValue(char *KeyName)
{
   return 0;
}
#endif
static uint8_t Key3Scan(void)
{
	return HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_4);
}
static uint8_t Key4Scan(void)
{
	return HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3);
}
static uint8_t Key5Scan(void)
{
	return HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_5);
}


/********************************************************************************
* 模块名称：独立按键驱动模块
* 功能说明：扫描独立按键，具有软件滤波机制，具有按键FIFO。可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
* 日期：2018.08.15
********************************************************************************/

/* 为每个按键定义一个结构体变量 */
static KEY_T 	s_key3;		//结构体变量
static KEY_T 	s_key4;
static KEY_T 	s_key5;
static KEY_T    s_key34;
//static KEY_T 	s_key6;
//static KEY_T 	s_key01;	//按键0/1组合按键， 结构体变量


/* 按键FIFO的结构体变量 */
static KEY_FIFO_T 	s_tKey;


/********************************************************************************
*函数名  ： IsKeyDownX
*功能说明： 判断按键是否按下
*输入参数： 无
*返回值  ： 1 表示按下；	0表示未按下；
*备注    ： 移植时注意修改
********************************************************************************/
static uint8_t IsKeyDown_key3(void) 	{if(Key3Scan() == 0) return 1;else return 0;}
static uint8_t IsKeyDown_key4(void)	 	{if(Key4Scan() == 0) return 1;else return 0;}
static uint8_t IsKeyDown_key5(void) 	{if(Key5Scan() == 0) return 1;else return 0;}
static uint8_t IsKeyDown_Key34(void) {if(Key3Scan() == 0 && Key4Scan() == 0)return 1;else return 0;}
/* 按键0/1的组合键 */
//static uint8_t IsKeyDown_01(void) {if(IsKeyDown_key0() && IsKeyDown_key1()) return 1;else return 0;}


/********************************************************************************
*	函 数 名: bsp_InitKeyVar
*	功能说明: 初始化按键变量
*	形    参:  无
*	返 回 值: 无
********************************************************************************/
static void bsp_InitKeyVar(void)
{
	/* 对按键FIFO读写指针清零 */
	s_tKey.Read = 0;
	s_tKey.Write = 0;


	/* 初始化按键 3 变量，支持按下 */
	s_key3.IsKeyDownFunc = IsKeyDown_key3;	//判断按键按下的函数
	s_key3.FilterTime = KEY_FILTER_TIME;	//按键滤波时间
	s_key3.LongTime = 20;		//需要触发长按的时间
	s_key3.Count  = s_key3.FilterTime >>1;	//滤波器计数器。此处的设置无意义。
	s_key3.State = 0;						//按键缺省状态，0为未按下
	s_key3.KeyCodeDown = KEY_3_DOWN;		//按键按下的键值代码；0表示不检测按键按下
	s_key3.KeyCodeUp = KEY_3_UP;					//按键弹起的键值代码；0表示不检测按键弹起
	s_key3.KeyCodeLong = KEY_3_LONG;					//按键被持续长按按下的键值代码；0表示不检测按键长按
	s_key3.RepeatSpeed = 50;					//按键连发的速度，即x个时间单位触发一次连按。
											//0为不支持连发。（如要检测连发，则长按也必须被支持）
	s_key3.RepeatCount = 0;					//连发计数器。此项初始值都为0


	/* 初始化按键 4 变量，支持按下 */
	s_key4.IsKeyDownFunc = IsKeyDown_key4;	//判断按键按下的函数
	s_key4.FilterTime = KEY_FILTER_TIME;	//按键滤波时间
	s_key4.LongTime = 20;		//需要触发长按的时间
	s_key4.Count  = s_key4.FilterTime >>1;	//滤波器计数器。此处的设置无意义。
	s_key4.State = 0;						//按键缺省状态，0为未按下
	s_key4.KeyCodeDown = KEY_4_DOWN;		//按键按下的键值代码；0表示不检测按键按下
	s_key4.KeyCodeUp = KEY_4_UP;					//按键弹起的键值代码；0表示不检测按键弹起
	s_key4.KeyCodeLong = KEY_4_LONG;					//按键被持续长按按下的键值代码；0表示不检测按键长按
	s_key4.RepeatSpeed = 50;					//按键连发的速度，即x个时间单位触发一次连按。
											//0为不支持连发。（如要检测连发，则长按也必须被支持）
	s_key4.RepeatCount = 0;					//连发计数器。此项初始值都为0


	/* 初始化按键 5 变量，支持按下 */
	s_key5.IsKeyDownFunc = IsKeyDown_key5;	//判断按键按下的函数
	s_key5.FilterTime = KEY_FILTER_TIME;	//按键滤波时间
	s_key5.LongTime = 20;		//需要触发长按的时间
	s_key5.Count  = s_key5.FilterTime >>1;	//滤波器计数器。此处的设置无意义。
	s_key5.State = 0;						//按键缺省状态，0为未按下
	s_key5.KeyCodeDown = KEY_5_DOWN;		//按键按下的键值代码；0表示不检测按键按下
	s_key5.KeyCodeUp = KEY_5_UP;					//按键弹起的键值代码；0表示不检测按键弹起
	s_key5.KeyCodeLong = KEY_5_LONG;					//按键被持续长按按下的键值代码；0表示不检测按键长按
	s_key5.RepeatSpeed = 50;					//按键连发的速度，即x个时间单位触发一次连按。
											//0为不支持连发。（如要检测连发，则长按也必须被支持）
	s_key5.RepeatCount = 0;					//连发计数器。此项初始值都为0

    /* 初始化按键 5 变量，支持按下 */
	s_key34.IsKeyDownFunc = IsKeyDown_Key34;	//判断按键按下的函数
	s_key34.FilterTime = KEY_FILTER_TIME;	//按键滤波时间
	s_key34.LongTime = 20;		//需要触发长按的时间
	s_key34.Count  = s_key34.FilterTime >>1;	//滤波器计数器。此处的设置无意义。
	s_key34.State = 0;						//按键缺省状态，0为未按下
	s_key34.KeyCodeDown = KEY_34_DOWN;		//按键按下的键值代码；0表示不检测按键按下
	s_key34.KeyCodeUp = KEY_34_UP;					//按键弹起的键值代码；0表示不检测按键弹起
	s_key34.KeyCodeLong = KEY_34_LONG;					//按键被持续长按按下的键值代码；0表示不检测按键长按
	s_key34.RepeatSpeed = 50;					//按键连发的速度，即x个时间单位触发一次连按。
											//0为不支持连发。（如要检测连发，则长按也必须被支持）
	s_key34.RepeatCount = 0;					//连发计数器。此项初始值都为0

}


/********************************************************************************
*	函 数 名: bsp_InitKeyHard
*	功能说明: 配置按键对应的GPIO
*	形    参:  无
*	返 回 值: 无
********************************************************************************/
static void bsp_InitKeyHard(void)
{
	#if 0
	//如果使用STM32CubeMX配置，则此处可不再写相关配置
	   GPIO_InitTypeDef GPIO_InitStructure;

    /*开启按键GPIO口的时钟*/
    Enable_Key_Clock();

    /*选择按键的引脚*/
    GPIO_InitStructure.Pin = Key_Right_Pin;
    /*设置引脚为输入模式*/
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    /*设置引脚不上拉也不下拉*/
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    /*使用上面的结构体初始化按键*/
    HAL_GPIO_Init(Key_Right_Group, &GPIO_InitStructure);


    /*选择按键的引脚*/
    GPIO_InitStructure.Pin = Key_Left_Pin;
    /*使用上面的结构体初始化按键*/
    HAL_GPIO_Init(Key_Left_Group, &GPIO_InitStructure);

	  /*选择按键的引脚*/
    GPIO_InitStructure.Pin = Key_Up_Pin;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
    /*使用上面的结构体初始化按键*/
    HAL_GPIO_Init(Key_Up_Group, &GPIO_InitStructure);
	#endif
	;
}



/********************************************************************************
*	函 数 名: bsp_InitKey
*	功能说明: 初始化按键。
*	形    参:  无
*	返 回 值: 无
*	备注：该函数需在FreeRTOS的开始任务中调用，以初始化按键。
********************************************************************************/
void bsp_InitKey(void)
{
	bsp_InitKeyVar();		/* 初始化按键变量 */

	bsp_InitKeyHard();		/* 初始化按键硬件 */

	bsp_GetKey();
  bsp_ClearKey();
	
	key.keyLeft =   KEY_3_UP;
	key.keyMiddle = KEY_4_UP;
	key.keyRight =  KEY_5_UP;
}


/********************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值写入按键FIFO缓冲区。
*	形    参:  _KeyCode : 按键代码
*	返 回 值: 无
*	备注：此函数可用于模拟一个按键。
********************************************************************************/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_tKey.Buf[s_tKey.Write] = _KeyCode;
	if(++s_tKey.Write >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/********************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参:  无
*	返 回 值: 按键代码
********************************************************************************/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/********************************************************************************
*	函 数 名: bsp_GetKeyState
*	功能说明: 读取按键的状态。结构体中的state值
*	形    参:  _ucKeyID : 按键ID，在枚举中定义。
*	返 回 值: 1 表示按下， 0 表示未按下
*	备注：读取的是结构体中state的状态值。
********************************************************************************/
uint8_t bsp_GetKeyState(uint8_t _ucKeyID)
{
	uint8_t ucState = 0;

	switch(_ucKeyID)
	{
		case KID_K3://枚举中的按键ID
			ucState = s_key3.State;
			break;

		case KID_K4://枚举中的按键ID
			ucState = s_key4.State;
			break;

		case KID_K5://枚举中的按键ID
			ucState = s_key5.State;
			break;

	    case KID_K6://枚举中的按键ID
			ucState = s_key34.State;
			break;

	}
	return ucState;
}


/********************************************************************************
*	函 数 名: bsp_GetKeyState
*	功能说明: 清空按键FIFO缓冲区
*	形    参: 无
*	返 回 值: 无
********************************************************************************/
void bsp_ClearKey(void)
{
	s_tKey.Write = s_tKey.Read;
}


/********************************************************************************
*	函 数 名: bsp_DetectKey
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参:  按键结构变量指针
*	返 回 值: 无
********************************************************************************/
static void bsp_DetectKey(KEY_T *_pBtn)
{

	/*
		如果没有初始化按键函数，则报错
		if (s_tBtn[i].IsKeyDownFunc == 0)
		{
			printf("Fault : DetectButton(), s_tBtn[i].IsKeyDownFunc undefine");
		}
	*/


	if (_pBtn->IsKeyDownFunc())		//如果按键被按下了
	{
		/* 下面的if/else if 为按下去抖滤波 */
		if (_pBtn->Count < _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count < 2 * _pBtn->FilterTime)
		{
			_pBtn->Count++;
		}
		else	//去抖结束
		{
			if (_pBtn->State == 0)	//如果当前的按键状态是未按下
			{
				_pBtn->State = 1;	//标记为按键按下状态

				/* 发送按钮按下的消息 */
				if(_pBtn->KeyCodeDown > 0)	//如果设置了KeyCodeDown的值，且不为0。（说明了开启了按下检测）
				{
					bsp_PutKey(_pBtn->KeyCodeDown);//将对应的按键按下的键值放入按键FIFO
				}
			}

			if (_pBtn->LongTime > 0)	//如果设置了LongTime的值，且不为0。（说明了开启了长按检测）
			{
				if (_pBtn->LongCount < _pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++_pBtn->LongCount == _pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						bsp_PutKey(_pBtn->KeyCodeLong);//将对应的按键长按的键值放入按键FIFO
					}
				}
				else
				{
					if (_pBtn->RepeatSpeed > 0)	//如果设置了RepeatSpeed的值，且不为0。（说明了开启了连发检测）
					{
						if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)//触发了连按
						{
							_pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							bsp_PutKey(_pBtn->KeyCodeDown);
						}
					}
				}
			}
		}
	}
	else	//按键未被按下，或者按键被松开了
	{
		/* 下面的if/else if 为按键弹起去抖滤波 */
		if(_pBtn->Count > _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count != 0)
		{
			_pBtn->Count--;
		}
		else	//Count ==0
		{
			if (_pBtn->State == 1)//如果之前按键的状态是被按下的
			{
				_pBtn->State = 0;//将按键状态标记为未按下

				/* 发送按钮弹起的消息 */
				if(_pBtn->KeyCodeUp > 0)//如果设置了KeyCodeUp的值，且不为0。（说明了开启了按键弹起检测）
				{
					bsp_PutKey(_pBtn->KeyCodeUp);//将对应按键弹起的键值放入按键FIFO */
				}
			}
		}

		_pBtn->LongCount = 0;
		_pBtn->RepeatCount = 0;
	}
}



/********************************************************************************
*	函 数 名: bsp_KeyScan
*	功能说明: 扫描所有按键。非阻塞，需被任务周期调用
*	形    参:  无
*	返 回 值: 无
********************************************************************************/
 

BSP_KEY *getKeyHandle(void)
{ 
   return &key;
}

void bsp_KeyScan(void)
{
	bsp_DetectKey(&s_key3);
	bsp_DetectKey(&s_key4);
	bsp_DetectKey(&s_key5);
	//bsp_DetectKey(&s_key34);
}
 
static bool shutdown_pending = false;        // 关机挂起标志
static uint8_t keyEntery;

bool getShutdownPending(void)
{ 
   return shutdown_pending;
}
uint8_t getKeyEntery(void)
{ 
   return keyEntery;
}
void updata_key_value(void)
{
	  extern TimerHandle_t xShutdownTimer;
    _DEV_CFG *cfgFile = getCfgFileHandle(); 
    static uint8_t lastKey = 0,shutdownTick = 0;
    lastKey = cfgFile->defaultKeyUI;    
    // 按键扫描
    bsp_KeyScan();
    uint32_t mkey = bsp_GetKey();    
    if (mkey != KEY_NONE) {
        switch (mkey) {
            case KEY_3_DOWN:					                    
						    if(!getRunState())
								{ 
									if (cfgFile->defaultKeyUI == 0)
                    cfgFile->defaultKeyUI = cfgFile->UInum - 1;
									else
                    cfgFile->defaultKeyUI--;								   
								}
								else
								{ 
								  key.keyLeft = KEY_3_DOWN;
								}
                break;
                    
            case KEY_3_UP:
                key.keyLeft = KEY_3_UP;
								shutdownTick = 0;
                break;
                
            case KEY_3_LONG:
                key.keyLeft = KEY_3_LONG;
                break;
                
            case KEY_4_DOWN:
                keyEntery = KEY_4_DOWN;
                key.keyMiddle = KEY_4_DOWN;
                if (shutdown_pending) {
									  shutdownTick++;
										player_button_music();
                    if( shutdownTick > 2)
                    { 
											extern void power_close(void);
											power_close();	
										  while(1){  extern void iwdg_feed(void);iwdg_feed();}
										}																	
                }
                break;
                
            case KEY_4_UP:
                keyEntery = KEY_4_UP;
                key.keyMiddle = KEY_4_UP;  
                shutdown_pending = false;
    						shutdownTick = 0;					
                break;
                
            case KEY_4_LONG:
                keyEntery = KEY_4_LONG;
                key.keyMiddle = KEY_4_LONG;
								
                shutdown_pending = true;
                break;
                
            case KEY_5_DOWN:               
						    if(!getRunState())
								{ 
	                if (cfgFile->defaultKeyUI == (cfgFile->UInum - 1))
                    cfgFile->defaultKeyUI = 0;
									else
                    cfgFile->defaultKeyUI++;							   
								}
								else
								{ 
								 key.keyRight = KEY_5_DOWN;
								}
                break;
                
            case KEY_5_UP:
                key.keyRight = KEY_5_UP;
								shutdownTick = 0;
                break;
                
            case KEY_5_LONG:
                key.keyRight = KEY_5_LONG;
                break;
                
            case KEY_34_LONG:
                break;
        }
        bsp_ClearKey();
    }
 
    
    if (lastKey != cfgFile->defaultKeyUI) {
        lastKey = cfgFile->defaultKeyUI;
			  if(!getRunState())                  
					SET_EVENT_GROUP(EVENT_REFRESH_MATRIX);
                    //SET_EVENT_GROUP_ISR(EVENT_PLAYER_KEY_VIOC); 
    }
		else if(mkey == KEY_4_UP)
				SET_EVENT_GROUP(EVENT_KEY_ENTERY);
}

 




