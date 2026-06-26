#ifndef __RAWMATRIX_H
#define __RAWMATRIX_H
#include "sys.h"

#define REMOTRE_LOGO 54
#define BLUE_LOGO    53
#define BAT_LOGO     55

void StartLogo(void);
void PowerStopLogo(void);
void redrawUIInit(char*filename,bool state);
void redrawMatrixUI(char *filename);
void refreshdefaultKeyUi(char *filename);
void sendUIlistNumber(uint8_t id);
 
uint8_t get_ui_num(void);
uint8_t get_ui_file(uint8_t key);
uint8_t getCurrentUiList(void);
uint8_t getDefaultKey(void);
#endif
