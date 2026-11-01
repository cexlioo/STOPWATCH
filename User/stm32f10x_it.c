#include "stm32f10x_conf.h"

#include "led.h"
#include "key.h"
#include "system.h"
#include "usart.h"
#include "at_M100p.h"
#include "bike.h"
#include "stdio.h"

extern TaskHandle_t at_task_handle;

void cmd_revevied(uint8_t rx_data);

/* 外部中断EXTI0的中断处理函数
 * External Interrupt 0 Interrupt Request Handler
 */
void EXTI0_IRQHandler(void)
{
		BaseType_t taskWoken = pdFALSE;
    /* 判断是 EXTI_Line0 产生中断 */
    if(EXTI_GetITStatus(KEY1_EXTI_LINE) != RESET) 
    {
        /* 按键按下，即 GPIOB 的 0 脚被拉低 */
        if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0)
        {    
            xTaskNotifyFromISR(bike_task_handle, NOTIFY_KEY1_PRESS, eSetBits, &taskWoken);
						portYIELD_FROM_ISR(taskWoken);
        }
        /* 清除中断标志位 */
        EXTI_ClearITPendingBit(KEY1_EXTI_LINE);     
    }
}
/* 外部中断 EXTI15_10 的中断处理函数
 * External Interrupt 0 Interrupt Request Handler
 */
void EXTI15_10_IRQHandler(void)
{
		BaseType_t taskWoken = pdFALSE;
    /* 判断是 EXTI_Line14 产生中断
     * 该函数会检测 EXTI中【中断等待寄存器，Pending register】对应位是否置位
     */
    if(EXTI_GetITStatus(KEY2_EXTI_LINE) != RESET) 
    {
        /* 按键按下，即 GPIOC 的 14 脚被拉低 */
        if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0)
        {    
            xTaskNotifyFromISR(bike_task_handle, NOTIFY_KEY2_PRESS, eSetBits, &taskWoken);
						portYIELD_FROM_ISR(taskWoken);
        }
        /* 清除【中断等待寄存器，Pending register】中的中断标志位 */
        EXTI_ClearITPendingBit(KEY2_EXTI_LINE);
    }
}

void USART1_IRQHandler(void)
{
  uint8_t rx_data;
  /* 判断 USART1 是否产生接受中断 */
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
    rx_data = USART_ReceiveData(USART1);
    cmd_revevied(rx_data);
    /* 清除 USART1 产生的接受中断 */
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

extern void at_receive_byte(char byte);
//串口中断处理函数
void USART2_IRQHandler(void)
{
	uint8_t rx_data;
	BaseType_t taskWoken = pdFALSE;
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		rx_data = USART_ReceiveData(USART2);
		at_receive_byte(rx_data);
		
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
	
	//IDEL中断，串口空闲，接收一帧完成
	if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
	{
		//重要：读SR再读DR清除IDEL标志
		volatile uint32_t temp;
		temp = USART2->SR;
		temp = USART2->DR;
		(void)temp;//防止编译器警告
		
		//通知任务进行数据处理
		xTaskNotifyFromISR(at_task_handle, NOTIFY_USART_RX, eSetBits, &taskWoken);
		portYIELD_FROM_ISR(taskWoken);
		
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);
	}
}

void cmd_revevied(uint8_t rx_data)
{
	#define RX_BUFFER_SIZE 16
	static char rx_buffer[RX_BUFFER_SIZE];
	static uint16_t rx_index = 0;
	BaseType_t taskWoken = pdFALSE;
	
	rx_buffer[rx_index++] = rx_data;
	
	if(rx_data == '\n')
	{
		rx_buffer[rx_index] = '\0';
		
		if(strncmp(rx_buffer, "get data", 8) == 0)
		{
			xTaskNotifyFromISR(bike_task_handle, NOTIFY_DATA_LIST, eSetBits, &taskWoken);
		}
		else
		{
			bike_data_export_request(rx_buffer);
			xTaskNotifyFromISR(bike_task_handle, NOTIFY_DATA_EXPORT, eSetBits, &taskWoken);
		}
		
		portYIELD_FROM_ISR(taskWoken);
		rx_index = 0;//复位索引
	}
}

extern void gps_data_received(uint8_t rx_data);
//串口中断处理函数
void USART3_IRQHandler(void)
{
	uint8_t rx_data;
	if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
	{
		rx_data = USART_ReceiveData(USART3);
		gps_data_received(rx_data);
		
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}



