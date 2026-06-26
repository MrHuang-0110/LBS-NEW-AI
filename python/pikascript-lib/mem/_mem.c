#include "mem.h"
#include "_mem.h"
#include "PikaVM.h"

 
pika_float _mem_acc(PikaObj *self, pika_float direction)
{ 
	DEV_MEM *mem = getMemHandle();
 
 
	
	if(direction < 3.0f)
			return mem->acceleration_mg[(uint8_t)direction];
	return 0.0f;
}
pika_float _mem_attitude(PikaObj *self, char* name)
{ 
	DEV_MEM *mem = getMemHandle();
 
 
    float pitch = mem->pitch;
    float roll = mem->roll;
 
    #define COMPARE_LTE 0   
    #define COMPARE_GTE 1  
 
    typedef struct {
        const char* name;
        float value;
        float threshold;
        int compare_type;
    } DirectionConfig;
    
 
    DirectionConfig config[] = {
        {"down",  pitch, -50.0f, COMPARE_LTE},
        {"up",    pitch,  50.0f, COMPARE_GTE},
        {"left",  roll,  -50.0f, COMPARE_LTE},
        {"right", roll,   50.0f, COMPARE_GTE}
    };
 
    for (int i = 0; i < sizeof(config)/sizeof(config[0]); i++) {
        if (strcmp(name, config[i].name) == 0) {
            switch (config[i].compare_type) {
                case COMPARE_LTE:
                    return (config[i].value <= config[i].threshold) ? 1.0 : 0.0;
                case COMPARE_GTE:
                    return (config[i].value >= config[i].threshold) ? 1.0 : 0.0;
                default:
                    return 0.0;
            }
        }
    }
    return 0.0;
}
pika_float _mem_gencory(PikaObj *self, pika_float direction)
{ 
	DEV_MEM *mem = getMemHandle();
 
	
  switch((uint8_t)direction)
	{ 
	  case 0:
			return mem->yaw;
		case 1:
			return mem->pitch;
		case 2:
			return mem->roll;
	}
	return 0.0f;
}
void _mem_restyaw(PikaObj *self)
{ 
    DEV_MEM *mem = getMemHandle();
    mem->is_refresh = false;
    while(!mem->is_refresh){pika_GIL_EXIT();vTaskDelay(1);pika_GIL_ENTER();}   
     resetyaw();
}

