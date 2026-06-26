#include "os.h"


void SuspendedPikaThoneTaskHandleByName(void)
{
     UBaseType_t uxArraySize, x;
    TaskStatus_t *pxTaskStatusArray;
    uint32_t ulTotalRunTime;
    TaskHandle_t xCurrentTaskHandle = xTaskGetCurrentTaskHandle();
    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if(pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,&ulTotalRunTime);
        for(x = 0; x < uxArraySize; x++) {
            if(strcmp("pika_thread", pxTaskStatusArray[x].pcTaskName) == 0) {
				if(xCurrentTaskHandle != pxTaskStatusArray[x].xHandle)
                      vTaskSuspend(pxTaskStatusArray[x].xHandle);
            }

        }
		vTaskSuspend(xCurrentTaskHandle);
        vPortFree(pxTaskStatusArray);
    }
}

void RecoverPikaThoneTaskHandleByName(void)
{
     UBaseType_t uxArraySize, x;
    TaskStatus_t *pxTaskStatusArray;
    uint32_t ulTotalRunTime;

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if(pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,&ulTotalRunTime);
        for(x = 0; x < uxArraySize; x++) {
            if(strcmp("pika_thread", pxTaskStatusArray[x].pcTaskName) == 0) {
               vTaskResume(pxTaskStatusArray[x].xHandle);
            }
        }
        vPortFree(pxTaskStatusArray);
    }
}

void SuspendedOTherPikaThoneTaskHandleByName(void)
{
     UBaseType_t uxArraySize, x;
    TaskStatus_t *pxTaskStatusArray;
    uint32_t ulTotalRunTime;
    TaskHandle_t xCurrentTaskHandle = xTaskGetCurrentTaskHandle();

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if(pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,&ulTotalRunTime);
        for(x = 0; x < uxArraySize; x++) {
            if(strcmp("pika_thread", pxTaskStatusArray[x].pcTaskName) == 0) {
				if(xCurrentTaskHandle != pxTaskStatusArray[x].xHandle)
                      vTaskSuspend(pxTaskStatusArray[x].xHandle);
            }
        }
        vPortFree(pxTaskStatusArray);
    }
}
