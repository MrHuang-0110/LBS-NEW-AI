#include "dataStruct.h"
//#include "malloc.h"
#include "ultrasion.h"
#include "touch.h"
#include "motor.h"
#include "color.h"
#include "gray.h"
#include "nfc_car.h"
#include "camer.h"
#include "grayv2.h"

bool identify_and_bind(__PORT *manager,uint8_t devId,uint8_t index) {
	
	   if(manager->sensors!=NULL)
		 {
			  manager->sensors= NULL;
		 }
		 if(index > 8)
			 return false;
 
		  switch(devId)
		{
		  case DEV_ID_ULTRASION:
				manager->sensors = (SensorBase*)create_ultrasion(index);
			  if(manager->sensors!=NULL)
				{ 
					manager->sensors->setParam = setAck;
				}
				else
				  return false;				
			break;
			
			case DEV_ID_TOUCH:
				manager->sensors = (SensorBase*)create_touch(index);
			  if(manager->sensors!=NULL)
				{ 
					manager->sensors->setParam = setAck;				 
				}
				else
				  return false;				
			break;
			
			case DEV_ID_BIG_MOTOR:
				manager->sensors = (SensorBase*)create_big_motor(index);
			  if(manager->sensors!=NULL)
				{ 
				  manager->sensors->setParam = setAck;		
				}
				else
				  return false;			   				
			break;
			
			case DEV_ID_SMALL_Motor:
				manager->sensors = (SensorBase*)create_small_motor(index);
			  if(manager->sensors!=NULL)
				{ 
					manager->sensors->setParam = setAck;						   
				}
				else
				  return false; 				
			break;
			
			#if DEV_ID_KT_STATE
			case DEV_ID_KT_Motor:
				manager->sensors = (SensorBase*)create_kt_motor(index);
			  if(manager->sensors!=NULL)
				{ 
				  manager->sensors->setParam = setAck;		
				}	
				else
				  return false;									
			break;
			#endif
			
			case DEV_ID_COLOR:
				manager->sensors = (SensorBase*)create_color(index);
			   if(manager->sensors!=NULL)
				 { 
				    manager->sensors->setParam = setAck;	
				 }
				else
				  return false; 
			break;
			
			case DEV_ID_GRAY:
				manager->sensors = (SensorBase*)create_gray(index);
			  if(manager->sensors!=NULL)
				{
					manager->sensors->setParam = setAck;	
				}		
				else
				  return false;					
			break;

			case DEV_ID_GRAY_V2:
				manager->sensors = (SensorBase*)create_gray_v2(index);
			  if(manager->sensors!=NULL)
				{
					manager->sensors->setParam = setAck;	
				}		
				else
				  return false;					
			break;
				
			case DEV_ID_NFC:
				manager->sensors = (SensorBase*)create_nfc_car(index);
			  if(manager->sensors!=NULL)
				{
					manager->sensors->setParam = setAck;		
				}
				else
				  return false;				
			break;
			
			case DEV_ID_CAMER:
				 
				manager->sensors = (SensorBase*)create_camer(index);
		    if(manager->sensors!=NULL)
				{
					manager->sensors->setParam = setAck;
				}	
				else
					return false; 		
		  break;
			/*....*/
		}
		return true;
}

void setAck(void* self, uint8_t *data)
{ 
   BaseType_t pxHigherPriorityTaskWoken;
 
	 SensorBase *base = (SensorBase*)self;
	  memset(base->data, 0, sizeof(base->data));
	 if(base->type == DEV_ID_GRAY_V2 || base->type == DEV_ID_BIG_MOTOR || base->type == DEV_ID_SMALL_Motor || base->type == DEV_ID_COLOR || base->type == DEV_ID_CAMER)
	 {
		   memcpy(base->data, data,sizeof(base->data));
	 }
	 else
	 {
		 strncpy((char*)base->data, (const char*)data, sizeof(base->data)-1);
		 base->data[sizeof(base->data)-1] = '\0';      
	 }
	 UartDeviceContext_t *pDev = &uartDevices[base->devId];
	 xQueueSendFromISR(pDev->devControlQueue, &base, &pxHigherPriorityTaskWoken);	
}
void set_sensor_parameter(SensorBase* sensor,uint8_t *param) {
	  
    if (sensor == NULL) {       
        return;
    }
    
    if (sensor->setParam != NULL) {
        sensor->setParam(sensor, param);		  
    }
}

 
