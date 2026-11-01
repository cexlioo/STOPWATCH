#include "key.h"

static void NVIC_Key_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* 配置NVIC为0号分组方式 */
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    /* 中断通道号是 EXTI0_IRQn */
    NVIC_InitStructure.NVIC_IRQChannel = KEY1_IRQChannel;
    /* 配置抢占优先级 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    /* 配置子优先级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* 使能中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* 中断通道号是 EXTI15_10_IRQn */
    NVIC_InitStructure.NVIC_IRQChannel = KEY2_IRQChannel;
    /* 配置抢占优先级 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    /* 配置子优先级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* 使能中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void EXTI_Key_Config(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;                                                     
    /* 将 EXTI 的 0 号中断线映射到 B0 脚 */
    GPIO_EXTILineConfig(KEY1_PORTSOURCE, KEY1_PINSOURCE); 
    /* 中断线选择 0 号 */
    EXTI_InitStructure.EXTI_Line = KEY1_EXTI_LINE;
    /* EXTI模式设置为中断模式 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 下降沿触发中断 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    /* 使能中断 */        
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
     /* 将 EXTI 的 14 号中断线映射到 C14 脚 */
    GPIO_EXTILineConfig(KEY2_PORTSOURCE, KEY2_PINSOURCE); 
    /* 中断线选择 14 号 */
    EXTI_InitStructure.EXTI_Line = KEY2_EXTI_LINE;
    /* EXTI模式设置为中断模式 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 下降沿触发中断 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    /* 使能中断 */        
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void key_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能外设 GPIOB 和 IO选择器 AFIO 的时钟 */
    RCC_APB2PeriphClockCmd(KEY1_CLK, ENABLE);
    /* 使能外设 GPIOC 和 IO选择器 AFIO 的时钟 */
    RCC_APB2PeriphClockCmd(KEY2_CLK, ENABLE);
   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = KEY2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY2_PORT, &GPIO_InitStructure);

    EXTI_Key_Config();
    NVIC_Key_Config();
}
