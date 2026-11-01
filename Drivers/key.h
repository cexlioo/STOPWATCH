#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x_conf.h "

#define KEY1_PORT          GPIOB                       /* GPIO端口组 */
#define KEY1_PIN           GPIO_Pin_0                  /* GPIO端口号 */
#define KEY1_CLK           (RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO)        /* GPIO端口时钟 */
#define KEY1_IRQChannel    EXTI0_IRQn                  /* 中断通道号 */
#define KEY1_PORTSOURCE    GPIO_PortSourceGPIOB        /* 外部中断端口组 */
#define KEY1_PINSOURCE     GPIO_PinSource0             /* 外部中断端口号 */
#define KEY1_EXTI_LINE     EXTI_Line0                  /* 外部中断线号 */

#define KEY2_PORT          GPIOC                       /* GPIO端口组 */
#define KEY2_PIN           GPIO_Pin_14                 /* GPIO端口号 */
#define KEY2_CLK           (RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO)        /* GPIO端口时钟 */
#define KEY2_IRQChannel    EXTI15_10_IRQn              /* 中断通道号 */
#define KEY2_PORTSOURCE    GPIO_PortSourceGPIOC        /* 外部中断端口组 */
#define KEY2_PINSOURCE     GPIO_PinSource14            /* 外部中断端口号 */
#define KEY2_EXTI_LINE     EXTI_Line14                 /* 外部中断线号 */

void key_init(void);

#endif
