#include "_gray.h"
#include "gray.h"
#include "motor.h"
#include "PikaVM.h"
#include "portagree.h"


void _gray_find_line_init(PikaObj *self)
{ 
	;
}
pika_float _gray_line(PikaObj *self, int port, int line)
{ 
	DEV_GRAY *gray = read_gray((SensorBase *)getDevBase(port));
  int value = 0; 
  
	if(gray == NULL)
		   return 0;
	
		if(line == 1)
			value = gray->L1;
		else if(line == 2)
			value = gray->L2;
		else if(line == 3)
			value = gray->R1;
		else if(line == 4)
			value = gray->R2;	
		
  return (float)value;
}
pika_float _gray_singe_line_state(PikaObj *self, int port, pika_float num)
{ 
	DEV_GRAY *gray = read_gray((SensorBase *)getDevBase(port));
  int value = 0; 

		if(gray == NULL)
		   return 0;
		
		if(num == 1)
			value = gray->L1State;
		else if(num == 2)
			value = gray->L2State;
		else if(num == 3)
			value = gray->R1State;
		else if(num == 4)
			value = gray->R2State;	

  return value;
}
pika_float _gray_single_state(PikaObj *self, int port)
{ 
	DEV_GRAY *gray = read_gray((SensorBase *)getDevBase(port));
  int value = 0; 

		if(gray == NULL)
		   return 0;
		
		switch(port)
		{
		  case 1:
				return gray->L1State;
			case 2:
				return gray->L2State;
			case 3:
				return gray->R1State;
			case 4:
				return gray->R2State;
		}
    return 0;
}
pika_float _gray_cmp_state(PikaObj *self, int port, pika_float num)
{ 
	DEV_GRAY *gray = read_gray((SensorBase *)getDevBase(port));
  int value = 0; 

		if(gray == NULL)
		   return 0;
		
     value = (gray->L1State<<3|gray->L2State<<2|gray->R1State<<1|gray->R2State<<0);

	if(value == (int)num)
  return 1;
	else
	return 0;
}
