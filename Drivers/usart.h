#ifndef __USART_H
#define __USART_H

#include "stm32f10x_conf.h"
#include <stdio.h>

#define USART1_PORT         GPIOA                   /* USART1 端口组 */
#define USART1_CLK          (RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1)    /* USART1 端口时钟 */
#define USART1_TX_PIN       GPIO_Pin_9              /* USART1 端口号 */
#define USART1_RX_PIN       GPIO_Pin_10             /* USART1 端口号 */

#define USART2_PORT					GPIOA
#define USART2_CLK					RCC_APB1Periph_USART2
#define USART2_TX_PIN				GPIO_Pin_2
#define USART2_RX_PIN				GPIO_Pin_3

#define USART3_PORT					GPIOB
#define USART3_CLK					(RCC_APB1Periph_USART3)
#define USART3_TX_PIN				GPIO_Pin_10
#define USART3_RX_PIN				GPIO_Pin_11

void usart1_send_byte(uint8_t byte);
void usart1_send_array(uint8_t* hex_array, uint8_t size);
void usart1_send_string(char* str);
void usart1_init(void);

void usart2_send_byte(uint8_t byte);
void usart2_send_array(uint8_t* hex_array,uint8_t size);
void usart2_send_string(char* str);
void usart2_init(void);

void usart3_send_byte(uint8_t byte);
void usart3_send_array(uint8_t* hex_array,uint8_t size);
void usart3_send_string(char* str);
void usart3_init(void);

#endif
