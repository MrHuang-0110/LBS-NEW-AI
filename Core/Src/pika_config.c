#include "pika_config.h"
//#include "malloc.h"
#include "devfile.h"
#include "PikaObj.h"
#include "PikaVM.h"
void pika_platform_free(void* ptr)
{
	  vPortFree(ptr);
}
void* pika_platform_malloc(size_t size)
{
   return pvPortMalloc(size);
}
void* pika_platform_realloc(void* ptr, size_t size)
{
	extern void *pvPortRealloc( uint8_t *srcaddr,size_t xWantedSize );
	return pvPortRealloc(ptr,size);
}

void pika_platform_sleep_ms(uint32_t ms) {
    
	extern bool getRunState(void);
	 pika_GIL_EXIT();
	while(VMSignal_getCtrl()!= VM_SIGNAL_CTRL_EXIT && ms > 0)
	{
		 vTaskDelay(1);
	   ms--;
	}
	 pika_GIL_ENTER();
}

int pika_platform_fclose(FILE* stream)
{
   _IO_FILE *file = getFatfsHandle();
	
   FRESULT fr = f_close(&file->ioFile);

   return (fr == FR_OK) ? 0 : -1;
}

size_t pika_platform_fwrite(const void* ptr,size_t size,size_t n,FILE* stream)
{
    UINT numBytesWritten;
	   _IO_FILE *file = getFatfsHandle();
    FRESULT fr = f_write(&file->ioFile, ptr, size * n, &numBytesWritten); // 写入数据
    if (fr != FR_OK) {
        return 0; // 写入失败返回0
    }
    return numBytesWritten; // 返回写入的字节数
}

 size_t pika_platform_fread(void* ptr,size_t size,size_t n,FILE* stream)

{
    UINT num_read;
	  _IO_FILE *file = getFatfsHandle();
    FRESULT res = f_read(&file->ioFile, ptr, size * n, &num_read);
    if (res != FR_OK) return 0;  // 错误处理可以根据需要调整
    return num_read;
}

int pika_platform_fseek(FILE* stream, long offset, int whence)
{
	   FRESULT res;
	  _IO_FILE *file = getFatfsHandle();
    if(whence == SEEK_SET)
        res = f_lseek(&file->ioFile, offset);
	  else if(whence == SEEK_END)
	      res = f_lseek(&file->ioFile, offset + file->ioFile.obj.objsize);

    return (res == FR_OK) ? 0 : -1;  // 返回0表示成功，-1表示失败
}
long pika_platform_ftell(FILE* stream)
{
	 _IO_FILE *file = getFatfsHandle();
	return f_tell(&file->ioFile);
}
 
FILE* pika_platform_fopen(const char* filename, const char* modes)
{
    uint8_t res;
   _IO_FILE *file = getFatfsHandle();
	if(strcmp(modes,"rb") == 0)
	  res = f_open(&file->ioFile,filename,FA_READ);
	else if(strcmp(modes,"wb+") == 0)
	  res = f_open(&file->ioFile,filename, FA_CREATE_ALWAYS|FA_WRITE);

	if(res == FR_OK) 
		return (FILE*)&file->ioFile;
	else
		return NULL;
	

}
void newai_Task_platformGetTick(PikaObj *self)
{ 
	obj_setInt(self, "tick", HAL_GetTick());
}
