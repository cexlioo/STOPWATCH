#include "led.h"

void led_red_on(void)
{
    GPIO_ResetBits(LED_RED_PORT, LED_RED_PIN);
}

void led_red_off(void)
{
    GPIO_SetBits(LED_RED_PORT, LED_RED_PIN);
}

void led_red_toggle(void) 
{
    /* 读取引脚的输出状态，并进行翻转，函数说明参考手册127页
     * 如果引脚时高，说明LED灯是灭的
     */
    if (GPIO_ReadOutputDataBit(LED_RED_PORT, LED_RED_PIN) == 1) {
        led_red_on();
    } else {
        led_red_off();
    }
}

void led_green_on(void)
{
    GPIO_SetBits(LED_GREEN_PORT, LED_GREEN_PIN);
}

void led_green_off(void)
{
    GPIO_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN);
}

void led_green_toggle(void) 
{
    /* 读取引脚的输出状态，并进行翻转，函数说明参考手册127页
     * 绿灯引脚和红灯相反，如果引脚是高，说明LED灯是亮的
     */
    if (GPIO_ReadOutputDataBit(LED_GREEN_PORT, LED_GREEN_PIN) == 1) {
        led_green_off();
    } else {
        led_green_on();
    }
}

void led_init(void)
{      
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(LED_RED_CLK, ENABLE); 
    RCC_APB2PeriphClockCmd(LED_GREEN_CLK, ENABLE); 

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN;
    GPIO_Init(LED_RED_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LED_GREEN_PIN;
    GPIO_Init(LED_GREEN_PORT, &GPIO_InitStructure);
    
    led_red_off();
    led_green_off();
}
