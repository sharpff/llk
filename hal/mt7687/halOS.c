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
int halLockInit(void *ptr, const char *file, int line) {
    //SemaphoreHandle_t m_mutex;

    // Semaphore cannot be used before a call to xSemaphoreCreateMutex().
    // This is a macro so pass the variable in directly.
    g_m_mutex = xSemaphoreCreateMutex();   
	
    printf("halLockInit() g_m_mutex:%p\n",  g_m_mutex);
    if (g_m_mutex != NULL) 
	{
        // The semaphore was created successfully.
        // The semaphore can now be used.
        return 0;
    }
    else 
        return -1;
	
}

int halLock(void *ptr, const char *file, int line) {
    if (NULL == g_m_mutex)
    {
        //assert(0);
        printf("halLock() g_m_mutex is NULL. \n");
		return -1;   
    }

    printf("[Lelink]mutex_lock() g_m_mutex:%p\n",  g_m_mutex);

    if (pdTRUE == xSemaphoreTake(g_m_mutex, portMAX_DELAY))
        return 0;
    else
        return -1;    
}

int halUnlock(void *ptr, const char *file, int line) {
    if (NULL == g_m_mutex)
    {
        //assert(0);
        printf("halUnlock() g_m_mutex is NULL. \n");
		return -1;  
    }

    printf("[Lelink]mutex_unlock() m_mutex:%p\n",  g_m_mutex);

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

void *halMalloc(size_t size) {
    void *ptr = pvPortMalloc(size);
    return ptr;
}

void *halCalloc(int n, size_t size) {
    void *ptr = pvPortMalloc(n*size);
    if (ptr) {
        os_memset(ptr, 0x00, n*size);
    }
    return ptr;
}

void *halRealloc(void *ptr, size_t size) {
    void *ptr1 = pvPortRealloc(ptr, size);
    return ptr1;
}

void halFree(void *ptr) {
    if (ptr)
        vPortFree(ptr);
}

int halReboot(void) {
    printf("Reboot Bye Bye Bye!!!!\n");
	hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
	
    //return 1;
    return 0;
}

uint16_t halRand() {
    return (uint16_t)os_random();
    //return 0;
}

void halDelayms(int ms) {
	 vTaskDelay(ms / portTICK_PERIOD_MS);
}