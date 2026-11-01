#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "stm32f10x_conf.h"
#include "stdio.h"
#include "FreeRTOS.h"       // FreeRTOS 主头文件，使用FreeRTOS必须包含
#include "task.h"           // FreeRTOS 任务相关头文件
#include "semphr.h"         // FreeRTOS 信号量头文件
#include "queue.h"          
#include "timers.h"

#include "log.h"

void os_free(void* ptr);
void* os_malloc(size_t size);
void os_mem_info(void);

void os_delay(int n);
void os_delay_ms(uint32_t ms);
void os_delay_s(uint32_t s);
void os_task_yield(void);

uint32_t os_millisFromISR(void);
uint32_t os_millis(void);

#endif
