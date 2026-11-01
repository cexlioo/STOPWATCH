#include "system.h"

/**
* @brief 获取系统启动的毫秒数，注意该函数只能在任务中调用
*/
uint32_t os_millis(void)
{
    TickType_t ticks = xTaskGetTickCount();
    return (uint32_t)(ticks * 1000 / configTICK_RATE_HZ);
}
/**
* @brief 获取系统启动的毫秒数，该函数可以在中断中调用
*/
uint32_t os_millisFromISR(void)
{
    TickType_t ticks = xTaskGetTickCountFromISR();
    return (uint32_t)(ticks * 1000 / configTICK_RATE_HZ);
}

//打印当前系统剩余动态内存大小
void os_mem_info(void)
{
	Log_d("Total_mem:%d freeMem:%d\n", configTOTAL_HEAP_SIZE, xPortGetFreeHeapSize());
}

//动态分配内存
void* os_malloc(size_t size)
{
	return pvPortMalloc(size);
}

//释放由os_malloc分配的内存块
void os_free(void* ptr)
{
	vPortFree(ptr);
}

void os_delay(int n) 
{
    int i;
    for (; n > 0; n--)
    for (i = 0; i < 2000; i++) ;
}

/**
  * @brief  毫秒级延时
  * @param  ms 延时时长，范围：0~4294967295
  * @retval 无
  */
void os_delay_ms(uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}
/**
  * @brief  秒级延时
  * @param  s 延时时长，范围：0~4294967295
  * @retval 无
  */
void os_delay_s(uint32_t s)
{
	while(s--)
	{
		os_delay_ms(1000);
	}
}

/**
* @brief  将CPU控制权交给下一个就绪任务
*/
void os_task_yield(void)
{
  taskYIELD();
}
