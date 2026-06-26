#include "_color.h"
#include "color.h"
#include "portagree.h"
#include "PikaVM.h"


pika_float _color_cmp_color(PikaObj *self, int port, pika_float number)
{ 
	DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));
	if(color == NULL)return 0;
	

		float h = color->hsv.h;
		float s = color->hsv.s;
		float v = color->hsv.v;	
		
		
		if((int)matchingcolor(h,s,v)->number == (int)number)
			return 1;
		else
			return 0;	   
 
}
pika_float _color_cmp_hsv(PikaObj *self, int port, pika_float rmin, pika_float rmax, pika_float gmin, pika_float gmax, pika_float bmin, pika_float bmax)
{ 
	DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));
  if(color == NULL)return 0;
	

		int r = color->RGBL[0];
		int g = color->RGBL[1];
		int b = color->RGBL[2];  

		
		if((r > rmin && r< rmax)&&
		 (g > gmin && g< gmax)&&
	   (b > bmin && b< bmax))
		{
			return 1;
		}
		else
		{
			return 0;
		}			

}
pika_float _color_cmp_lux(PikaObj *self, int port, char* judgment, pika_float value)
{ 
  DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));
  if(color == NULL)return 0;
	int resualt = 0;

   switch(*judgment)
	 { 
	   case '<':
			 resualt = (value < color->RGBL[3])?1.0f:0.0f;
		 break;
		 case '>':
			 resualt = (value > color->RGBL[3])?1.0f:0.0f;
		 break;
		 case '=':
			 resualt = (value == color->RGBL[3])?1.0f:0.0f;
		 break;
		 default:
			 resualt = 0;
		   break;
	 }

  return resualt;
}
pika_float _color_dectection(PikaObj *self, int port)
{ 
	DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));
  if(color == NULL)return 0;
	

		float h = color->hsv.h;
		float s = color->hsv.s;
		float v = color->hsv.v;	  
		
		return matchingcolor(h,s,v)->number;

}
pika_float _color_lux(PikaObj *self, int port)
{ 
  DEV_COLOR *color = read_color((SensorBase *)getDevBase(port));
  if(color == NULL)return 0;


		return color->RGBL[3];

}
pika_float _color_lux_black_line(PikaObj *self, int port, pika_float value)
{ 
  return 0;
}
pika_float _color_rgb(PikaObj *self, int port, pika_float color)
{ 
	  DEV_COLOR *dev_color = read_color((SensorBase *)getDevBase(port));
    if(dev_color == NULL)return 0;
	  
	  float rgb;

		  if(color == 1)
				  rgb = dev_color->RGBL[0];
			else if(color == 4)
				  rgb = dev_color->RGBL[1];
			else if(color == 3)
				  rgb = dev_color->RGBL[2];
			
    return rgb;
}
