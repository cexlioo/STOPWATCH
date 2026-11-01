#include "stm32f10x_conf.h"

#include "led.h"
#include "usart.h"
#include "system.h"
#include "i2c.h"
#include "OLED.h"
#include "linked_queue.h"
#include "at_M100p.h"
#include "bike.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	//禁止中断
	portDISABLE_INTERRUPTS();
	//打印溢出的任务名
	printf("\n%s overflow\n", pcTaskName);
	//无限循环，防止继续执行
	while(1);
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	log_init(LOG_DEBUG);
	
	bike_init();
	
  printf("FreeRTOS system init for TTBike\r\n");
	
	bike_start();
	/* GPS接收，要在 bike task 启动后 */
	usart3_init();
	
  // 开启任务调度器                        
  vTaskStartScheduler();
  printf("FreeRTOS system failed!! NEVER BEEN HERE！\n");
}
