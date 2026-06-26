#ifndef __MATRIX_H
#define __MATRIX_H
#include "sys.h"

 
#define X_POS_SIZE 9
#define Y_POS_SIZE 7
#define x_point X_POS_SIZE
#define y_point Y_POS_SIZE
#define _Matrix_H 58
#define _Matrix_L 9

#define UI_DEFAULT_COLOR     0x080FF00
#define UI_DEFAULT_BRIGTNESS 12

#define USER_DEFAULT_COLOR     0x080FF00
#define USER_DEFAULT_BRIGTNESS 12

#define BLUE_SET_ON_COLOR    0x0000FF
#define BLUE_SET_OFF_COLOR   UI_DEFAULT_COLOR
typedef struct
{
	 char lamp[9];
	 unsigned int RollDelayTime;
	 unsigned int xy[9][7];
	 unsigned int color,xycolor;
	 unsigned int brightness,xybrightness;
	 unsigned char state;
}_API_MATRIX_CFG;

typedef struct
{ 
	unsigned char lamp[9];
	unsigned int xy[9][7];
  unsigned int color,xycolor;
	unsigned int brightness,xybrightness;
}DRIVER_MATRIX;

const unsigned char *getBatLevel(uint8_t level);
void loadMatrixResource(void);
unsigned int CalculatedBrightness(int color,unsigned char brightness);
void _show_write_led(unsigned char *lamp,unsigned int color);
void _show_roll_ui(uint8_t num);
void _show_roll_pika(char *text,uint32_t length);
void _ui_row_refresh(void);
void _clear_matrix(void);
void refresmatrtixlamp(uint8_t state,uint8_t PortIndex);
void matrix_set_xy_color(int color);
void matrix_set_pixe(uint8_t x,uint8_t y);
void showError(uint8_t PortIndex);
void showOK(uint8_t PortIndex);
void RollingLed(uint8_t State,uint8_t loop,uint8_t y,int color);
void python_matrix_init(void);
DRIVER_MATRIX *getmatrixhandle(void);
_API_MATRIX_CFG *get_api_matrix_handle(void);
#endif
