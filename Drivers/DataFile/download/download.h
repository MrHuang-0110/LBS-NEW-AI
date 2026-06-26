#ifndef __DOWNLOAD_H
#define __DOWNLOAD_H
#include "sys.h"
#include "portagree.h"
#include "ringbufer.h"
#include "matchineState.h"
 
#define USB_PORT  0x09
#define BLUE_PORT 0x0A

enum
{ 
  STATE_CREATE_FILE = 0x0,
	STATE_RECEIVE_DATA,
	STATE_RECEIVE_END,
	STATE_SAVE_FILE,
};
 
typedef struct
{ 
  char path[32],name[32];
	size_t objSize;
  uint8_t *fileData;
}DOWNLOAD_FILE;

typedef struct
{ 
	 uint8_t STATE;
   DOWNLOAD_FILE dfile; 
}NEWAI_FILE;

void initdownloadResource(void);
RingBuffer *getUSB_RingBuffer_Handle(void);
RingBuffer *getBLUE_RingBuffer_Handle(void);
RingBuffer *getPROCES_RingBuffer_Handle(void);
void downloadFile(uint8_t port,RingBuffer *q);
 
#endif

