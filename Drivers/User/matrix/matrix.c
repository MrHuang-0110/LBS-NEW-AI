#include "matrix.h"
#include "delay.h"
//#include "malloc.h"
#include "string.h"
#include "rawMatrix.h"
#include "PikaObj.h"
#include "PikaVM.h"
extern SPI_HandleTypeDef hspi1;
static _API_MATRIX_CFG api_matrix;
static DRIVER_MATRIX py_matrix;

//static __attribute__((section (".RAM_D2"))) char LAMP_CACHE[2*1024];
//static __attribute__((section (".RAM_D2"))) char VALISt_CACHE[512];
//static __attribute__((section (".RAM_D2"))) char SHOW_ROLL_CACHE[512];
 
static char __attribute__((section (".DMA_SRAM2_16KB"))) lamp_cache[2*1024];
static char __attribute__((section (".DMA_SRAM2_16KB"))) valist_cache[512];
static char __attribute__((section (".DMA_SRAM2_16KB"))) show_roll_cache[512];

const unsigned char Font8X8[_Matrix_H][_Matrix_L]={

{0x00,0x7C,0x44,0x44,0x44,0x44,0x44,0x7C,0x00},//0
{0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x7C,0x00},//1
{0x00,0x7C,0x04,0x04,0x7C,0x40,0x40,0x7C,0x00},//2
{0x00,0x7C,0x04,0x04,0x7C,0x04,0x04,0x7C,0x00},//3
{0x00,0x44,0x44,0x44,0x44,0x7C,0x04,0x04,0x00},//4
{0x00,0x7C,0x40,0x40,0x7C,0x04,0x04,0x7C,0x00},//5
{0x00,0x7C,0x40,0x40,0x7C,0x44,0x44,0x7C,0x00},//6
{0x00,0x7C,0x04,0x08,0x10,0x20,0x40,0x40,0x00},//7
{0x00,0x7C,0x44,0x44,0x7C,0x44,0x44,0x7C,0x00},//8
{0x00,0x7C,0x44,0x44,0x7C,0x04,0x04,0x7C,0x00},//9
{0x00,0x5C,0x54,0x54,0x54,0x54,0x54,0x5C,0x00},//10
{0x00,0x24,0x6C,0x24,0x24,0x24,0x24,0x24,0x00},//11
{0x00,0x5C,0x44,0x44,0x5C,0x50,0x50,0x5C,0x00},//12
{0x00,0x5C,0x44,0x44,0x5C,0x44,0x44,0x5C,0x00},//13
{0x00,0x54,0x54,0x54,0x5C,0x44,0x44,0x44,0x00},//14
{0x00,0x5C,0x50,0x50,0x5C,0x44,0x44,0x5C,0x00},//15
{0x00,0x5C,0x50,0x50,0x5C,0x54,0x54,0x5C,0x00},//16
{0x00,0x5C,0x44,0x48,0x50,0x50,0x50,0x50,0x00},//17
{0x00,0x5C,0x54,0x54,0x5C,0x54,0x54,0x5C,0x00},//18
{0x00,0x5C,0x54,0x54,0x5C,0x44,0x44,0x5C,0x00},//19
{0x00,0x10,0x28,0x44,0x44,0x7C,0x44,0x44,0x00},/*"A",0*/
{0x00,0x78,0x44,0x44,0x7C,0x44,0x44,0x78,0x00},/*"B",1*/
{0x00,0x7C,0x40,0x40,0x40,0x40,0x40,0x7C,0x00},/*"C",2*/
{0x00,0x78,0x44,0x44,0x44,0x44,0x44,0x78,0x00},/*"D",3*/
{0x00,0x7C,0x40,0x40,0x7C,0x40,0x40,0x7C,0x00},/*"E",4*/
{0x00,0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x00},/*"F",5*/
{0x00,0x7C,0x40,0x40,0x4C,0x44,0x44,0x7C,0x00},/*"G",6*/
{0x00,0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x00},/*"H",7*/
{0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x7C,0x00},/*"I",8*/
{0x00,0x3C,0x08,0x08,0x08,0x28,0x28,0x10,0x00},/*"J",9*/
{0x00,0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00},/*"K",10*/
{0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7C,0x00},/*"L",11*/
{0x00,0x44,0x6C,0x54,0x44,0x44,0x44,0x44,0x00},/*"M",12*/
{0x00,0x44,0x64,0x44,0x54,0x44,0x4C,0x44,0x00},/*"N",13*/
{0x00,0x38,0x44,0x44,0x44,0x44,0x44,0x38,0x00},/*"O",14*/
{0x00,0x78,0x44,0x44,0x78,0x40,0x40,0x40,0x00},/*"P",15*/
{0x00,0x38,0x44,0x44,0x44,0x44,0x4C,0x3E,0x00},/*"Q",16*/
{0x00,0x78,0x44,0x44,0x78,0x60,0x50,0x48,0x00},/*"R",17*/
{0x00,0x3C,0x40,0x40,0x7C,0x04,0x04,0x78,0x00},/*"S",18*/
{0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00},/*"T",19*/
{0x00,0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x00},/*"U",20*/
{0x00,0x44,0x44,0x44,0x44,0x44,0x28,0x10,0x00},/*"V",21*/
{0x00,0x54,0x54,0x54,0x54,0x54,0x6C,0x44,0x00},/*"W",22*/
{0x00,0x44,0x28,0x10,0x10,0x10,0x28,0x44,0x00},/*"X",23*/
{0x00,0x44,0x28,0x10,0x10,0x10,0x10,0x10,0x00},/*"Y",24*/
{0x00,0x7C,0x04,0x08,0x10,0x20,0x40,0x7C,0x00},/*"Z",25*/
{0x00,0x38,0x44,0x44,0x04,0x08,0x10,0x00,0x10},/*? 46*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00},/*. 47*/
{0x00,0x00,0x10,0x10,0x7C,0x10,0x10,0x00,0x00},/*+ 48*/
{0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00},/*- 49*/
{0x00,0x00,0x00,0x28,0x10,0x28,0x00,0x00,0x00},/** 50*/
{0x00,0x00,0x10,0x00,0x7C,0x00,0x10,0x00,0x00},/*/ 51*/
{0x00,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00},/*= 52*/
{0x00,0x18,0x54,0x34,0x18,0x14,0x34,0x58,0x00},/*blue 53*/
{0x00,0x38,0x7C,0x7C,0x38,0x10,0x10,0x7C,0x00},/*remote 54*/
{0x00,0x38,0x7C,0x44,0x44,0x44,0x44,0x7C,0x00},/*bat 55*/
{0x00,0x38,0x44,0x44,0x44,0x28,0x28,0x38,0x00},/*brightness 56*/
{0x00,0x28,0x7C,0x44,0x44,0x28,0x28,0x38,0x00},/*Bus 57*/
};

const unsigned char batLevel[4][9] = {
  {0x00,0x38,0x7C,0x44,0x44,0x44,0x7C,0x7C,0x00},/*0*/
  {0x00,0x38,0x7C,0x44,0x44,0x7C,0x7C,0x7C,0x00},/*1*/
	{0x00,0x38,0x7C,0x44,0x7C,0x7C,0x7C,0x7C,0x00},/*2*/
	{0x00,0x38,0x7C,0x7C,0x7C,0x7C,0x7C,0x7C,0x00},/*3*/
};

const unsigned char *getBatLevel(uint8_t level)
{
  return batLevel[level];	
}
unsigned int CalculatedBrightness(int color,unsigned char brightness)
{
   unsigned char R,G,B;

	  G = (color>>16&0xFF)*brightness/100.0f;
	  R = (color>>8&0xFF)*brightness/100.0f;
	  B = (color&0xFF)*brightness/100.0f;

	  return (G<<16)|(R<<8)|(B<<0);
}

 void _show_write_led(unsigned char *lamp,unsigned int color)
{
	   unsigned short i = 0, j = 0,k = 0;
	   unsigned int offset = 48;
	   unsigned char Point;
	   unsigned int	Lamp32Byte[x_point][y_point],TempColor;

		 uint8_t *pSRAM = (uint8_t*)lamp_cache;
		 memset(pSRAM,0,2*1024);
	
	   if(lamp == NULL)
			   return;

	   /*1.clear bufer data*/
     memset(Lamp32Byte,0,sizeof(Lamp32Byte));

			/*2.write color data*/
			for (i = 0; i < x_point; i++)
			{
			    Point =lamp[i];
				for (j = 0; j <y_point; j++)
				{

					 if (Point & 0x80)
					{
							Lamp32Byte[i][j] = color;
					}
					else
					{
							Lamp32Byte[i][j] = 0;
					}
					Point <<= 1;
				}
			}


		 if(api_matrix.state == 1)
		 {
				api_matrix.state = 0;
			  for (i = 0; i < x_point; i++)
			 {
				  for (j = 0; j < y_point; j++)
				{
			      Lamp32Byte[i][j] |= api_matrix.xy[i][j];
				}
			 }
		 }

			/*3.data - > DMA */
			for (i = 0; i < x_point; i++)
			{
				for (j = 0; j < y_point; j++)
				{
					TempColor = Lamp32Byte[i][j];
					for (k = 0; k < 24; k++)
					{
						/*zeor code*/
						if(TempColor&0x800000)
						{
							pSRAM[offset++] = 0xFC;
						}
						/*one code*/
						else
						{
							pSRAM[offset++] = 0xE0;
						}
						TempColor <<= 1;
					}
				}
			}

			/*4.send*/
		 	HAL_SPI_Transmit_DMA(&hspi1,pSRAM,(X_POS_SIZE * Y_POS_SIZE *24)+48);
			while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

}

static unsigned char reverseBits(unsigned char x) {
    unsigned char result = 0;
    unsigned char bitPos;
    for (bitPos = 0; bitPos < 8; bitPos++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}
static void _move_left(unsigned char *data,unsigned char length,unsigned char n)
{
     unsigned char i = 0,j = 0;

      for(i = 0;i<n;i++)
	  {
		  for(j = 0;j<length;j++)
		  {
			 data[j] = data[j+1];
			 data[j+1] = 0;
		  }
	  }
}
static void copy_row(unsigned char target[], unsigned char row, unsigned char cols)
{
    memcpy(target, Font8X8[row], cols * sizeof(unsigned char));
}

void _ui_row_refresh(void)
{ 
   uint8_t uiListNum = getCurrentUiList();
	 char lamp[9];
	 memset(lamp,0,sizeof(lamp));
	 memcpy(lamp,(const char*)Font8X8[uiListNum],9);
	 for(uint8_t i = 0;i<9;i++)
   { 
	   _move_left((unsigned char*)lamp,9,1);
		 vTaskDelay(10);
		  _show_write_led((unsigned char*)lamp,CalculatedBrightness(UI_DEFAULT_COLOR,UI_DEFAULT_BRIGTNESS));
	 }
}
 void _show_roll_ui(uint8_t num)
{
	  int color;
    uint8_t i,x,y;
	  char ShowBmp[9];
	  uint8_t *point = (uint8_t*)show_roll_cache;

	  memset(ShowBmp,0,sizeof(ShowBmp));
	  memset(show_roll_cache,0,512);
    
	  color = UI_DEFAULT_COLOR;
	  copy_row(point,num,9);
    if(num == BLUE_LOGO)
		{ 
			   #include "blue.h"			
		    if(blueHandle()->blueState == 0)
					color = BLUE_SET_OFF_COLOR; 	   
				else
					color = BLUE_SET_ON_COLOR;
		}
		/*ÄæÐò*/
		for(i = 0;i<9;i++)
	 {
			*(point+i) = reverseBits(*(point+i));
	 }

	 for(y = 0;y<7;y++)
	 {
			for(x = 0;x<9;x++)
		 {
				 ShowBmp[x]<<=1;
				 show_roll_cache[x]>>=1;
				 ShowBmp[x]|=show_roll_cache[x]&0x01;
			}
				 _show_write_led((unsigned char*)ShowBmp,CalculatedBrightness(color,UI_DEFAULT_BRIGTNESS));
	  		 delay_ms(25);
	 }
	 
	 
}
void _clear_matrix(void)
{ 
   unsigned char lamp[9];
	 memset(lamp,0,9);
	 _show_write_led(lamp,0);
	
	_API_MATRIX_CFG *api_handle = get_api_matrix_handle();
	memset(api_handle->lamp,0,9);
	
	for(uint8_t x = 0;x<9;x++)
  { 
	  for(uint8_t y= 0;y<7;y++)
		{
		  api_handle->xy[x][y] = 0;
		} 
	}
}
void matrix_set_xy_color(int color)
{
   api_matrix.xycolor = color;
}
void matrix_set_pixe(uint8_t x,uint8_t y)
{ 
 
	_API_MATRIX_CFG *api_handle = get_api_matrix_handle();
	
	if(x > 9)x=9;
	if(y > 7)y= 7;
	
  api_handle->xy[(int)x][(int)y] = CalculatedBrightness(api_handle->xycolor,api_handle->xybrightness);
	
	api_handle->state = 1;
 
	_show_write_led((unsigned char*)api_handle->lamp,CalculatedBrightness(api_handle->color,api_handle->brightness));
}
 
void refresmatrtixlamp(unsigned char state,unsigned char PortIndex)
{
 
     unsigned char _x_point[8] = {1,3,5,7,1,3,5,7};
     unsigned char _y_point[8] = {0,0,0,0,6,6,6,6};

    switch(state)
    {
        case 0:
        {
             matrix_set_xy_color(0);
            break;
        }
        case 1:
        {
            matrix_set_xy_color(UI_DEFAULT_BRIGTNESS);
            break;
        }
    }
				
		matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);
}
void showError(unsigned char PortIndex)
{
   unsigned char _x_point[8] = {1,3,5,7,1,3,5,7};
   unsigned char _y_point[8] = {0,0,0,0,6,6,6,6};

  if(_y_point[PortIndex] == 0)
  {

    matrix_set_xy_color(0);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]+1);

    matrix_set_xy_color(0);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]+2);

    matrix_set_xy_color(0x00FF00);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);
  }
  else if(_y_point[PortIndex] == 6)
  {
    matrix_set_xy_color(0);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex] - 1);

    matrix_set_xy_color(0);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex] - 2);

    matrix_set_xy_color(0x00FF00);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);
  }
}

void showOK(uint8_t PortIndex)
{
   uint8_t _x_point[8] = {1,3,5,7,1,3,5,7};
   uint8_t _y_point[8] = {0,0,0,0,6,6,6,6};

  if(_y_point[PortIndex] == 0)
  {
		_y_point[PortIndex] = 0;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);

		_y_point[PortIndex] =1;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);

		_y_point[PortIndex] = 2;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);
  }

  else if(_y_point[PortIndex] == 6)
  {
		_y_point[PortIndex] = 6;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);

		_y_point[PortIndex] = 5;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);

		_y_point[PortIndex] = 4;
		matrix_set_xy_color(0xFF0000);
    matrix_set_pixe(_x_point[PortIndex],_y_point[PortIndex]);
  }
}


void RollingLed(unsigned char State,unsigned char loop,unsigned char y,int color)
{
    unsigned char strX[8] = {1,3,5,7,1,3,5,7};
    matrix_set_xy_color(color);
    if(State ==1)
    {
       if(loop<4)
      {
        matrix_set_pixe(strX[loop],0+y);
      }
      else if(loop>=4 && loop<8)
     {
        matrix_set_pixe(strX[loop],6-y);
     }
    }
    else if(State == 0)
    {
       if(loop<4)
      {
        matrix_set_pixe(strX[loop],0+y);
      }
      else if(loop>=4 && loop<8)
     {
        matrix_set_pixe(strX[loop],6-y);
     }
    }
}
void _show_roll_pika(char *text,uint32_t length)
{
	  int color,teampColor;
      uint8_t loop,i,x,y;
	  char ShowBmp[9];
 
	  uint8_t *point;

	  memset(ShowBmp,0,sizeof(ShowBmp));
	  memset(show_roll_cache,0,512);
	  teampColor = py_matrix.color;
      color = CalculatedBrightness((int)teampColor,py_matrix.brightness);
      point = (uint8_t*)show_roll_cache;
      for(loop = 0;loop<length;loop++)
	  {
	      if(text[loop]>= '0' && text[loop]<= '9')
			   copy_row(point,text[loop] - 48,9);
		  else if(text[loop]>= 'A' && text[loop]<= 'Z')
				 copy_row(point,text[loop] - 45,9);
		  else if(text[loop] == '?')
				 copy_row(point,46,9);
		  else if(text[loop] == '.')
				 copy_row(point,47,9);
          else if(text[loop] == '+')
				 copy_row(point,48,9);
          else if(text[loop] == '-')
				 copy_row(point,49,9);
          else if(text[loop] == '*')
				 copy_row(point,50,9);
          else if(text[loop] == '/')
				 copy_row(point,51,9);
          else if(text[loop] == '=')
				 copy_row(point,52,9);
          else
				 copy_row(point,46,9);

			if(length > 1)
			{
		    for(i = 0;i<9;i++)
		    {
			    *(point+i) = reverseBits(*(point+i));
		    }
             point+=9;
			}
	  }

		if(length == 1)
		{
		   _show_write_led((unsigned char*)show_roll_cache,color);
		}
		else
		{
		  for(loop = 0;loop<length && VMSignal_getCtrl() != 1;loop++)
          {
            for(y = 0;y<Y_POS_SIZE && VMSignal_getCtrl() != 1;y++)
           {
              for(x = 0;x<X_POS_SIZE && VMSignal_getCtrl() != 1;x++)
            {
              ShowBmp[x]<<=1;
              show_roll_cache[(loop*9)+x]>>=1;
              ShowBmp[x] |= show_roll_cache[(loop*9)+x]&0x01;
            }
         
		     if( VMSignal_getCtrl())
		   	     return;

            _show_write_led((unsigned char*)ShowBmp,color);
			      pika_GIL_EXIT();
            vTaskDelay(api_matrix.RollDelayTime);
			      pika_GIL_ENTER();
           }
					 extern bool getRunState(void);
					 if(VMSignal_getCtrl())break;
		  }

		}
}
 
void loadMatrixResource(void)
{
   api_matrix.color = api_matrix.xycolor = USER_DEFAULT_COLOR;
   api_matrix.brightness = api_matrix.xybrightness = USER_DEFAULT_BRIGTNESS;
   api_matrix.state = 0;
   api_matrix.RollDelayTime = 100;
	
   /*
	 if(lamp_cache!=NULL &&
			valist_cache!=NULL &&
	    show_roll_cache!=NULL)
	 {
	   memset(lamp_cache,0,2*1024);
	   memset(valist_cache,0,512);
	   memset(show_roll_cache,0,512);
	   return;
	 }
    
	 
	 lamp_cache = (char *)mymalloc(SRAM12,2*1024);
	 valist_cache = (char *)mymalloc(SRAM12,512);
	 show_roll_cache = (char *)mymalloc(SRAM12,512);
	 
	 
	   memset(lamp_cache,0,2*1024);
	   memset(valist_cache,0,512);
	   memset(show_roll_cache,0,512);
		  */
		 /*
	  if(lamp_cache== NULL || 
			 valist_cache == NULL || 
		   show_roll_cache == NULL)
	  {
				 while(1);
	  }*/

	  memset(lamp_cache,0,2*1024);
	  memset(valist_cache,0,512);
	  memset(show_roll_cache,0,512);
}

void python_matrix_init(void)
{ 
  py_matrix.brightness = UI_DEFAULT_BRIGTNESS;
	py_matrix.color = UI_DEFAULT_COLOR;
	py_matrix.xybrightness = UI_DEFAULT_BRIGTNESS;
	py_matrix.xycolor = UI_DEFAULT_COLOR;
	
	memset(py_matrix.lamp,0,sizeof(py_matrix.lamp));
	
	for(unsigned x = 0;x<9;x++)
	{ 
	  for(unsigned y = 0;y<7;y++)
		{ 
		  py_matrix.xy[x][y] = 0;
			api_matrix.xy[x][y] = 0;
		}
	}
	memset(api_matrix.lamp,0,sizeof(api_matrix.lamp));
}
 
_API_MATRIX_CFG *get_api_matrix_handle(void)
{ 
  return &api_matrix;
}
DRIVER_MATRIX *getmatrixhandle(void)
{ 
   return &py_matrix;
}
