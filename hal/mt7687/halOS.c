#include "leconfig.h"
#include "halHeader.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "os_util.h"
#include "hal_sys.h"
#include "stdio.h"
#include "stddef.h"
#include "os.h"

SemaphoreHandle_t g_m_mutex = NULL;

extern void vPortFree(void* pv);
extern void* pvPortMalloc(size_t xWanteSize);
extern void *pvPortRealloc( void *pv, size_t size );  

int halLockInit(void) {
    //SemaphoreHandle_t m_mutex;

    // Semaphore cannot be used before a call to xSemaphoreCreateMutex().
    // This is a macro so pass the variable in directly.
    g_m_mutex = xSemaphoreCreateMutex();   
	
    APPLOG("halLockInit() g_m_mutex:%p",  g_m_mutex);
    if (g_m_mutex != NULL) {
        // The semaphore was created successfully.
        // The semaphore can now be used.
        return 0;
    }
    else 
        return -1;
	
}

int halLock(void) {
    if (NULL == g_m_mutex) {
        //assert(0);
        APPLOG("halLock() g_m_mutex is NULL.");
		return -1;   
    }

    //APPLOG("[Lelink]mutex_lock() g_m_mutex:%p",  g_m_mutex);

    if (pdTRUE == xSemaphoreTake(g_m_mutex, portMAX_DELAY))
        return 0;
    else
        return -1;    
}

int halUnlock(void) {
    if (NULL == g_m_mutex) {
        //assert(0);
        APPLOG("halUnlock() g_m_mutex is NULL.");
		return -1;  
    }

    //APPLOG("[Lelink]mutex_unlock() m_mutex:%p",  g_m_mutex);

    if (pdTRUE == xSemaphoreGive(g_m_mutex))
        return 0;
    else
        return -1;   
}

unsigned int halGetTimeStamp(void) {
    struct os_time t = {0};
	
	//return (unsigned int)time(NULL);
	os_get_time(&t);
	
	return (unsigned int)t.sec;
}

unsigned int halGetUTC(void) {
    return 1234;
}

void *halMallocEx(size_t size, char* filename, uint32_t line) {
    void *ptr = pvPortMalloc(size);
    //APPLOG("malloc:[%d][0x%x][%d][%s]", size, ptr, line, filename);
    if(ptr==NULL) {
        APPLOG("halMallocEx:%d, %d",  size, xPortGetFreeHeapSize());
    }
    return ptr;
}

void *halCallocEx(size_t n, size_t size, char* filename, uint32_t line) {
    void *ptr = pvPortMalloc(n*size);
    //APPLOG("calloc:[%d][%d][0x%x][%d][%s]", n*size,xPortGetFreeHeapSize(), ptr, line, filename);
    if(ptr==NULL) {
        APPLOG("halCallocEx:%d, %d",  n*size, xPortGetFreeHeapSize());
    }
    if (ptr) {
        os_memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halReallocEx(void *ptr, size_t size, char* filename, uint32_t line) {
    void *ptr1 = pvPortRealloc(ptr, size);
    //APPLOG("realloc:[%d][0x%x][%d][%s]", size, ptr1, line, filename);
    //APPLOG("realloc:[%d][0x%x]", size, ptr1);
    if (ptr1==NULL) {
        APPLOG("halReallocEx:%d, %d\n",  size,xPortGetFreeHeapSize());
    }
    return ptr1;
}

void halFreeEx(void *ptr, char* filename, uint32_t line) {
    //APPLOG("halFreeEx:[0x%x][%d][%s]", ptr, line, filename);
    if (ptr)
        vPortFree(ptr);
}

#if 0
void *_halMalloc(size_t size) {
    void *ptr = pvPortMalloc(size);
    if(ptr==NULL) {
        APPLOG("halMalloc:0x%x, left=0x%x",  size, xPortGetFreeHeapSize());
    }
    return ptr;
}

void *_halCalloc(int n, size_t size) {
    void *ptr = pvPortMalloc(n*size);
    if(ptr==NULL) {
        APPLOG("pvPortMalloc:0x%x, left=0x%x",  n*size, xPortGetFreeHeapSize());
    }
    if (ptr) {
        os_memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *_halRealloc(void *ptr, size_t size) {
    void *ptr1 = pvPortRealloc(ptr, size);
    if (ptr1==NULL) {
        APPLOG("halRealloc:0x%x, left=0x%x",  size, xPortGetFreeHeapSize());
    }
    return ptr1;
}

void _halFree(void *ptr) {
    if (ptr)
        vPortFree(ptr);
}
#endif

int halReboot(void) {
    APPLOG("Reboot Bye Bye Bye!!!!");
	hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
    return 0;
}

uint16_t halRand() {
    return (uint16_t)os_random();
}

void halDelayms(int ms) {
	 vTaskDelay(ms / portTICK_PERIOD_MS);
}

uint32_t halGetCurrentTaskId(void) {
    return (uint32_t)xTaskGetCurrentTaskHandle();
}

