#ifndef __LED_H
#define __LED_H

#include "stm32f10x_conf.h"

#define LED_RED_PORT            GPIOC                       /* GPIO端口组 */
#define LED_RED_CLK             RCC_APB2Periph_GPIOC        /* GPIO端口时钟 */
#define LED_RED_PIN             GPIO_Pin_13                  /* GPIO端口号 */

#define LED_GREEN_PORT          GPIOA                       /* GPIO端口组 */
#define LED_GREEN_CLK           RCC_APB2Periph_GPIOA        /* GPIO端口时钟 */
#define LED_GREEN_PIN           GPIO_Pin_0                  /* GPIO端口号 */

void led_green_off(void);
void led_green_on(void);
void led_green_toggle(void);

void led_red_off(void);
void led_red_on(void);
void led_red_toggle(void);

void led_init(void);

#endif
