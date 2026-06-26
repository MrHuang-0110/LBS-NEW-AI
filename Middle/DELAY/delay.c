#include "sys.h"
#include "delay.h"
static unsigned char fac_us=0;
static unsigned short fac_ms=0;
void delay_us(uint32_t nus)
{
	unsigned int ticks;
	unsigned int told,tnow,tcnt=0;
	unsigned int reload=SysTick->LOAD;
	ticks=nus*fac_us;
	told=SysTick->VAL;
	while(1)
	{
		tnow=SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;
		}
	};
}

void delay_ms(uint16_t nms)
{
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)
	{
		if(nms>=fac_ms)
		{
   			vTaskDelay(nms/fac_ms);
		}
		nms%=fac_ms;
	}
	delay_us((unsigned int)(nms*1000));
}

void delay_init(uint16_t sysclk)
{
	int reload;
 	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	fac_us=sysclk;
	reload=sysclk;
	reload*=1000000/configTICK_RATE_HZ;

	fac_ms=1000/configTICK_RATE_HZ;
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD=reload;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
}
