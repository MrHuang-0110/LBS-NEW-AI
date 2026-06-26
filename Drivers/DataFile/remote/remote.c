#include "remote.h"

static uint8_t remoteValue[16];

uint8_t remote_linke = 0;

void set_remote_linke(void)
{
  remote_linke = 1;  
}
void close_remote_linke(void)
{ 
  remote_linke = 0;
}
uint8_t get_remote_linke(void)
{ 
  return remote_linke;
}
uint8_t *getremotevalue(void)
{ 
   return remoteValue;
}
void refreshRemoteValue(uint8_t *data,uint8_t length)
{ 
   memcpy(remoteValue,data,length); 
	 #if 0
   USB_printf("%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d\r\n",remoteValue[0],
	                                                   remoteValue[1],
	                                                   remoteValue[2],
	                                                   remoteValue[3],
	                                                   remoteValue[4],
	                                                   remoteValue[5],
	                                                   remoteValue[6],
	                                                   remoteValue[7],
	                                                   remoteValue[8],
	                                                   remoteValue[9],
	                                                   remoteValue[10],
																								     remoteValue[11],
	                                                   remoteValue[12],
	                                                   remoteValue[13],
	                                                   remoteValue[14],
	                                                   remoteValue[15]);	
	 #endif
}
void clearRemoteValue(void)
{ 
   memset(remoteValue,0,sizeof(remoteValue));
	 close_remote_linke();
}
