#ifndef __DEVFILE_H
#define __DEVFILE_H
#include "sys.h"
#include "music.h"
typedef struct
{
   FATFS fatfs;
   FIL   ioFile;
   DIR   ioDir;
   FILINFO ioinfo;
   uint8_t *FATFS_WORK;
}_IO_FILE;

typedef struct
{
	uint8_t  blue_state;
  uint32_t total,free;
	uint8_t  defaultKeyUI,UInum;
	uint16_t verBigMotor,verSmalMotor,verColor,verGray,verGrayV2,newAIversion;
}_DEV_CFG;

void fatfsInit(void);
void replace_extension(const char* original);
uint8_t check_file_extension(const char *filename);
char getFileObjSize(char *path,size_t *objSize);
char ExternFlashReadFile(char *path,uint8_t *pData);
char ExternFlashWrite(char *path,const char*data,int size);
_IO_FILE *getFatfsHandle(void);
_DEV_CFG *getCfgFileHandle(void);
void export_csv_to_uart(const char *filename);
void getFlashTotalFree(unsigned int *total,unsigned int *free);
void getNewAiSoftware(uint16_t *version);
void getdevSoftware(uint8_t id,uint16_t *version);
bool FATFS_MAKEFILE(void);
void closeFatfs(void);
uint8_t _read_sensord_bin(uint8_t index,uint8_t *data,uint32_t *fileSize);
bool _read_sensord_versionfile(uint8_t index,char *data);
int Delete_File(const char* filename);
int Delete_AllFile(void);
#endif
