#include "devfile.h"
#include "w25qxx.h"
#include "music.h"
#include "motor.h"
#include "color.h"
#include "gray.h"
//#include "malloc.h"
#include "rawMatrix.h"
#include "PikaCompiler.h"
#include "grayv2.h"
#include "blue.h"
static _IO_FILE file;
static _DEV_CFG cfgfile;
SemaphoreHandle_t xFsMutex = NULL;

void closeFatfs(void)
{ 
	 if(xFsMutex == NULL)return;
	
	 if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
	 { 
		  f_close(&file.ioFile);
	    xSemaphoreGive(xFsMutex);
	 }
    
}
 
int Delete_File(const char* filename)
{
	  char res;
	  if(xFsMutex == NULL)return 0;
	  if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
		{ 
			if (f_stat(filename, NULL) != FR_OK) {      
				xSemaphoreGive(xFsMutex);
        return -1;
			}
			res = f_unlink(filename);
			if (res != FR_OK) {
				xSemaphoreGive(xFsMutex);
        return res;
			}
			xSemaphoreGive(xFsMutex);
			return 0;	  
		}
    xSemaphoreGive(xFsMutex);
		return 0;	
}

int Delete_AllFile(void)
{ 
    char res,strfile[32];
	  if(xFsMutex == NULL)return 0;		
	  if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
		{ 
			  for(uint8_t i = 0;i<get_ui_num();i++)
			{
				  memset(strfile,0,sizeof(strfile));
				  sprintf(strfile,"1:app/%d.o",get_ui_file(i)); 
				  if (f_stat(strfile, NULL) != FR_OK)     
											continue;
				  res = f_unlink(strfile);
			} 
		}
    xSemaphoreGive(xFsMutex);
		return res;		
}
 uint8_t check_file_extension(const char *filename) {
    const char *extension = strrchr(filename, '.'); // 找到最后一个'.'
    if (extension != NULL) {
        if (strcmp(extension, ".py") == 0) {
            return 1; // 是.py文件
        } else if (strcmp(extension, ".o") == 0) {
            return 2; // 是.o文件
        }
    }
    return 0; // 不是.py或.o文件
}

void replace_extension(const char* original) {

    if (original == NULL) return;

    // 查找最后一个 `.` 的位置
    char* dot = strrchr(original, '.');
    if (dot != NULL) {
        *dot = '\0';  // 截断到扩展名前
    }
    // 追加新扩展名 `.o`
    strcat((char*)original, ".o");
}

static unsigned char get_io_getfree(unsigned char *drv,unsigned int *total,unsigned int *free)
{
	FATFS *fs1;
	unsigned char res;
  unsigned int fre_clust=0, fre_sect=0, tot_sect=0;
  if(xFsMutex == NULL)return res;
	if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
	{ 
    res =(unsigned int)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
		{
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;
	    fre_sect=fre_clust*fs1->csize;
	 
        #if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
        #endif
	 
		*total=tot_sect>>1;
		*free=fre_sect>>1;
		}
   	xSemaphoreGive(xFsMutex);
	}
	return res;
}
static void readSoftwareVersion(uint16_t *version,char *path)
{
   UINT bw;
   char res;
   char StrVersion[8];
   memset(StrVersion,0,sizeof(StrVersion));
	 if(xFsMutex == NULL)return;
	 if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
	 { 
			res = f_open(&file.ioFile,path,FA_READ);
			if(res!=FR_OK)
			{
        *version = 0;
        f_close(&file.ioFile);
				xSemaphoreGive(xFsMutex);
        return;
			}
			res = f_read(&file.ioFile,StrVersion,file.ioFile.obj.objsize,&bw);
			if(res!=FR_OK || bw!=file.ioFile.obj.objsize)
			{
      *version = 0;
      f_close(&file.ioFile);
			}
			*version = atoi(StrVersion);
			f_close(&file.ioFile);	
      xSemaphoreGive(xFsMutex);			
	 }
 
}

void getFlashTotalFree(unsigned int *total,unsigned int *free)
{ 
	 *total = cfgfile.total;
	 *free = cfgfile.free;
}
void getNewAiSoftware(uint16_t *version)
{ 
   *version = cfgfile.newAIversion;
}
void getdevSoftware(uint8_t id,uint16_t *version)
{ 
  switch(id)
 {
		case DEV_ID_BIG_MOTOR:
		*version = cfgfile.verBigMotor;
		break;
		
		case DEV_ID_SMALL_Motor:
		*version = cfgfile.verSmalMotor;
		break;
		
		case DEV_ID_COLOR:
		*version = cfgfile.verColor;
		break;
		
		case DEV_ID_GRAY:
		*version = cfgfile.verGray;
		break;
		
		case DEV_ID_GRAY_V2:
		*version = cfgfile.verGrayV2;
		break;
		default:
			*version = 0;
		break;		
 }
}
_IO_FILE *getFatfsHandle(void)
{ 
   return &file;
}
_DEV_CFG *getCfgFileHandle(void)
{ 
   return &cfgfile;
}

char getFileObjSize(char *path,size_t *objSize)
{ 
    char res = FR_DISK_ERR;
	  if(xFsMutex == NULL)return res;
	  if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
		{ 
		  res = f_open(&file.ioFile,path,FA_READ); 
			if(res != FR_OK)
			{
				f_close(&file.ioFile);
				xSemaphoreGive(xFsMutex);			
			  return res;
			}
		
			*objSize = file.ioFile.obj.objsize;
//			USB_printf("*objsize:%d\r\n",*objSize);
			f_close(&file.ioFile);
			xSemaphoreGive(xFsMutex);			
			return res;	   
		}
			xSemaphoreGive(xFsMutex);			
			return res;	   		
}

char ExternFlashReadFile(char *path,uint8_t *pData)
{ 
	   UINT bw;
     char res = FR_DISK_ERR;
	   if(xFsMutex == NULL)return res;
	   if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
		 { 
		 res = f_open(&file.ioFile,path,FA_READ);  
	   if(res != FR_OK)
		 {
		    f_close(&file.ioFile);
			  xSemaphoreGive(xFsMutex);			
		    return res;		
		 }
		 
		 memset(pData,0,file.ioFile.obj.objsize);
		 int sumSize = file.ioFile.obj.objsize;
		 do
		 {
				if(sumSize > 4096)
				{
				   res = f_read(&file.ioFile,(uint8_t*)pData,4096,&bw);
				   sumSize-=4096;
				   pData+=4096;
				}
				else
				{
				   res = f_read(&file.ioFile,(uint8_t*)pData,sumSize,&bw);
				   sumSize-=bw;
				}

				if(res!=FR_OK)
           break;
		 }while(sumSize);		 
	 
		 f_close(&file.ioFile);
		 xSemaphoreGive(xFsMutex);			
		 return res;		 		   
		}
		  xSemaphoreGive(xFsMutex);			
			return res;	   
}
char ExternFlashWrite(char *path,const char*data,int size)
{
	   UINT bw;
     char res = FR_DISK_ERR;
	   if(xFsMutex == NULL)return res;
	   if(xSemaphoreTake(xFsMutex, portMAX_DELAY) == pdTRUE)
		 { 
		 res = f_open(&file.ioFile,path,FA_CREATE_ALWAYS|FA_WRITE);
     if(res!=FR_OK)
     {
        f_close(&file.ioFile);
			 xSemaphoreGive(xFsMutex);		
				return res;
		  }
			 int sumSize = size;
			 do
			 {
				if(sumSize > 4096)
				{
				   res = f_write(&file.ioFile,(uint8_t*)data,4096,&bw);
				   sumSize-=4096;
				   data+=4096;
				}
				else
				{
				   res = f_write(&file.ioFile,(uint8_t*)data,sumSize,&bw);
				   sumSize-=bw;
				}

				if(res!=FR_OK)
            break;

			 }while(sumSize);
			 
       f_close(&file.ioFile);
			 xSemaphoreGive(xFsMutex);		
			  return res;		    
		 }
		 			xSemaphoreGive(xFsMutex);			
			return res;	   
 
}
void export_csv_to_uart(const char *filename)
{
    FIL file;
    FRESULT res;
    char line_buf[128];    // 每行缓冲区
    UINT bytes_read;

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        USB_printf("Failed to open file\r\n");
        return;
    }

    USB_printf("--- CSV file content start ---\r\n");

    while (f_gets(line_buf, sizeof(line_buf), &file) != NULL) {
        USB_printf(line_buf);   // 直接发送每一行
    }

    USB_printf("--- CSV file content end ---\r\n");

    f_close(&file);
}

uint8_t _read_sensord_bin(uint8_t index,uint8_t *data,uint32_t *fileSize)
{
    unsigned char res;
	  UINT br = 0;
		switch(index)
		{
					 case DEV_ID_BIG_MOTOR:
					 {
						  res = getFileObjSize("1:app/motor.bin",fileSize);
						  res = ExternFlashReadFile("1:app/motor.bin",data);
							break;
					 }
					 case DEV_ID_COLOR:
					 {
						  res = getFileObjSize("1:app/color.bin",fileSize);
						  res = ExternFlashReadFile("1:app/color.bin",data);
							break;
					 }
					 case DEV_ID_SMALL_Motor:
					 {
						  res = getFileObjSize("1:app/small_motor.bin",fileSize);
						  res = ExternFlashReadFile("1:app/small_motor.bin",data);
							break;
					 }
					 case DEV_ID_GRAY:
					 {
						  res = getFileObjSize("1:app/gray.bin",fileSize);
						  res = ExternFlashReadFile("1:app/gray.bin",data);
							break;
					 }
					 case DEV_ID_GRAY_V2:
					 { 
						  res = getFileObjSize("1:app/grayv2.bin",fileSize);
						  res = ExternFlashReadFile("1:app/grayv2.bin",data);						 
					    break;
					 }
            default:break;
	  }
		return res;
}
bool _read_sensord_versionfile(uint8_t index,char *data)
{ 
   _DEV_CFG *cfgfile = getCfgFileHandle();
	  switch(index)
		{
					 case DEV_ID_BIG_MOTOR:
					 {
						  sprintf((char*)data,"%d",cfgfile->verBigMotor);
							break;
					 }
					 case DEV_ID_COLOR:
					 {
						  sprintf((char*)data,"%d",cfgfile->verColor);
							break;
					 }
					 case DEV_ID_SMALL_Motor:
					 {
							sprintf((char*)data,"%d",cfgfile->verSmalMotor);
							break;
					 }
					 case DEV_ID_GRAY:
					 {
							sprintf((char*)data,"%d",cfgfile->verGray);
							break;
					 }
					 case DEV_ID_GRAY_V2:
					 {
						 sprintf((char*)data,"%d",cfgfile->verGrayV2);
					    break;
					 }
            default:break;
	  }	 
		return true;
}
bool FATFS_MAKEFILE(void)
{
	 unsigned char MyFile_Res;
	
	 if(file.FATFS_WORK!=NULL)
	 { 
	   //myfree(SRAMIN,file.FATFS_WORK);
		  vPortFree(file.FATFS_WORK);
	 }

	 //file.FATFS_WORK = mymalloc(SRAMIN,2*1024);
	 file.FATFS_WORK = pvPortMalloc(2*1024);
	 if(file.FATFS_WORK!=NULL)
		  memset(file.FATFS_WORK,0,2*1024);
   else
		  goto MAKFS_ERROR;
      
	 MyFile_Res = f_mount(&file.fatfs,"1:",0);
	 MyFile_Res = f_mkfs("1:",FM_FAT,0,file.FATFS_WORK ,sizeof 2*1024);
	 if (MyFile_Res == FR_OK)
	 {
	    MyFile_Res = f_mount(&file.fatfs,"1:",0);
		  if(MyFile_Res == FR_OK)
		  {
			  MyFile_Res = f_mount(&file.fatfs,"1:",1);
				if(MyFile_Res != FR_OK)
				{
					goto MAKFS_ERROR;
				}
		  }
		  else
		  {
				 goto MAKFS_ERROR;
		  }
	 }
	 else
	 {
		 goto MAKFS_ERROR;
	 }

	 return true;

   MAKFS_ERROR:
			return false;

}

void fatfsInit(void)
{ 
  unsigned char res;
  OSPI_W25Qxx_Init();
  int retry_count = 0;
    do {
        res = f_mount(&file.fatfs, "1:", 1);
        if (res == FR_OK) {
            if(get_io_getfree((unsigned char*)"1:", &cfgfile.total, &cfgfile.free) == FR_OK) {
                break; // 成功则退出循环
            }
        }
        retry_count++;
        vTaskDelay(10); // 短暂延迟后重试
    } while (retry_count < 3);
    
    if (res != FR_OK) {
        // 可以添加错误处理，如记录日志或显示错误信息
        return;
    }
		if (ExternFlashReadFile("1:system.cfg", (uint8_t*)&cfgfile) == 0) {
         BLUE_CONFIG *blue_handle = blueHandle();
			   blue_handle->blueState = cfgfile.blue_state;
		} else {
        memset((_DEV_CFG*)&cfgfile,0,sizeof(_DEV_CFG));
		}  
  readSoftwareVersion(&cfgfile.verBigMotor,"1:version/BigMotorVersion.txt");
	readSoftwareVersion(&cfgfile.verColor,"1:version/ColorVersion.txt");
	readSoftwareVersion(&cfgfile.verGray,"1:version/gray.txt");
	readSoftwareVersion(&cfgfile.verSmalMotor,"1:version/MiddleMotorVersion.txt");
	readSoftwareVersion(&cfgfile.verGrayV2,"1:version/grayv2.txt");
	readSoftwareVersion(&cfgfile.newAIversion,"1:version/Version.txt");
	
 
	/*init newai cofig file*/
	#if 0
	UINT br;
	res = f_open(&file.ioFile,"1:/config/systemfile.cfg",FA_READ);
	if(res!=FR_OK)
	{
     res = f_mkdir("1:config");
	   res = f_open(&file.ioFile,"1:/config/systemfile.cfg",FA_CREATE_ALWAYS|FA_WRITE);
	   if(res!=FR_OK)
	   {
	      f_close(&file.ioFile);
		    while(1);
	   }
     cfgfile.defaultKeyUI = 0;
		 cfgfile.UInum = 0;
	  
	   res = f_write(&file.ioFile,&cfgfile,sizeof(_DEV_CFG),&br);
	   if(res!=FR_OK)
	   {
		   while(1);
	   }

	   f_close(&file.ioFile);
		 return;
	}

	res = f_read(&file.ioFile,&cfgfile,sizeof(_DEV_CFG),&br);
	if(res != FR_OK)
	{
		while(1);
	}
	f_close(&file.ioFile);
	#endif
}


 
