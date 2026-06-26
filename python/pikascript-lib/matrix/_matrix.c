#include "_matrix.h"
#include "matrix.h"
#include "PikaVM.h"

static uint8_t temp_lamp[9];

void clear_matrix_cach(void)
{ 
   memset(temp_lamp,0,sizeof(temp_lamp));
}
void _matrix_clear(PikaObj *self)
{  
	 memset(temp_lamp,0,sizeof(temp_lamp));
	 _show_write_led(temp_lamp,0);
}
void _matrix_set_brightness(PikaObj *self, pika_float brigntness)
{ 
	DRIVER_MATRIX *handle = getmatrixhandle();
  float fbrigntness;
	fbrigntness = (brigntness / 100.0f)*10.0f;
  handle->brightness = fbrigntness>10?10:fbrigntness;
	_show_write_led((unsigned char*)temp_lamp,CalculatedBrightness((int)handle->color,handle->brightness));
}
void _matrix_set_color(PikaObj *self, pika_float color)
{ 
	DRIVER_MATRIX *handle = getmatrixhandle();
  handle->color = (unsigned int)color;
	_show_write_led((unsigned char*)temp_lamp,CalculatedBrightness((int)handle->color,handle->brightness));
}
void _matrix_set_pixel(PikaObj *self, pika_float x, pika_float y)
{ 
	DRIVER_MATRIX *handle = getmatrixhandle();
	_API_MATRIX_CFG *api_handle = get_api_matrix_handle();
	
	if(x > 9)x=9;
	if(y > 7)y= 7;
	
  api_handle->xy[(int)x][(int)y] = CalculatedBrightness(handle->xycolor,handle->xybrightness);
	
	api_handle->state = 1;
	
	_show_write_led((unsigned char*)handle->lamp,CalculatedBrightness(handle->color,handle->brightness));
}
void _matrix_set_pixel_brightness(PikaObj *self, pika_float x, pika_float y, pika_float brigntness)
{ 
  DRIVER_MATRIX *handle = getmatrixhandle();
	_API_MATRIX_CFG *api_handle = get_api_matrix_handle();
	float fbrigntness;
	
	fbrigntness = (brigntness / 100.0f)*10.0f;
	
	api_handle->xy[(int)x][(int)y] = CalculatedBrightness(handle->xycolor,fbrigntness>10?10:fbrigntness);
	api_handle->state = 1;
	_show_write_led((unsigned char*)handle->lamp,CalculatedBrightness(handle->color,handle->brightness));
}
void _matrix_show(PikaObj *self, pika_float color, pika_float bufer1, pika_float bufer2, pika_float bufer3, pika_float bufer4, pika_float bufer5, pika_float bufer6, pika_float bufer7, pika_float bufer8, pika_float bufer9)
{ 
	 DRIVER_MATRIX *handle = getmatrixhandle();
   memset(temp_lamp,0,sizeof(temp_lamp));
	
   temp_lamp[0] = (unsigned char)bufer1;
	 temp_lamp[1] = (unsigned char)bufer2;
	 temp_lamp[2] = (unsigned char)bufer3;
	 temp_lamp[3]=  (unsigned char)bufer4;
	 temp_lamp[4]=  (unsigned char)bufer5;
	 temp_lamp[5] = (unsigned char)bufer6;
	 temp_lamp[6] = (unsigned char)bufer7;
	 temp_lamp[7] = (unsigned char)bufer8;
	 temp_lamp[8] = (unsigned char)bufer9;
	
    _show_write_led((unsigned char*)temp_lamp,CalculatedBrightness((int)color,handle->brightness));
}
 
void _matrix_show_roll(PikaObj *self, char* text)
{ 
   char showStr[32];
   memset(showStr,0,sizeof(showStr));

   char *endptr;
   float value = strtod(text, &endptr);
    if (*endptr != '\0') {

      strcpy(showStr,text);
		 _show_roll_pika(showStr,strlen(showStr));
        return;
    }
    int intPart = (int)value;
    double fracPart = value - intPart;
    if (fabs(fracPart - round(fracPart * 100.0) / 100.0) < 1e-9) {

        sprintf(showStr, "%d", intPart);
    } else {
        sprintf(showStr, "%.2f", value);
        char *ptr = showStr + strlen(showStr) - 3;
        while (*ptr == '0') {
            *ptr = '\0';
            ptr--;
            if (*ptr == '.') {
                *ptr = '\0';
                break;
            }
        }
        if (strcmp(showStr, "") == 0) {
            sprintf(showStr, "%d", intPart);
        }
     }

      _show_roll_pika(showStr,strlen(showStr));
}
