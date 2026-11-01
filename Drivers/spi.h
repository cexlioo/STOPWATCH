#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x_conf.h"
#include "system.h"

#define __USE_HARD_SPI_INTERFACE__
//#define __USE_SOFT_SPI_INTERFACE__

#define SPIx						SPI1
#define SPI_CLK					(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1)
#define SPI_GPIO_CLK		RCC_APB2Periph_GPIOA
#define SPI_CLK_PORT		GPIOA
#define SPI_CLK_PIN			GPIO_Pin_5

#define SPI_MISO_PORT		GPIOA
#define SPI_MISO_PIN		GPIO_Pin_6

#define SPI_MOSI_PORT		GPIOA
#define SPI_MOSI_PIN		GPIO_Pin_7

//时钟拉高/拉低引脚
#define SPI_SCLK_WRITE(x)		GPIO_WriteBit(SPI_CLK_PORT,SPI_CLK_PIN,(BitAction)(x))
//主设备数据输出，从设备数据输入
#define SPI_MOSI_WRITE(x)		GPIO_WriteBit(SPI_MOSI_PORT,SPI_MOSI_PIN,(BitAction)(x))
//主设备数据输入，从设备数据输出
#define SPI_MISO_READ()			GPIO_ReadInputDataBit(SPI_MISO_PORT,SPI_MISO_PIN)

void spi_init(void);
uint8_t spi_read_write_byte(uint8_t byte_send);

#endif
