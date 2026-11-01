#include "spi.h"

#ifdef __USE_SOFT_SPI_INTERFACE__

void spi_init(void)
{
	//使能
	RCC_APB2PeriphClockCmd(SPI_GPIO_CLK,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = SPI_CLK_PIN | SPI_MOSI_PIN;
	GPIO_Init(SPI_CLK_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
	GPIO_Init(SPI_MISO_PORT,&GPIO_InitStructure);
	
	//模式0 低电平空闲
	SPI_SCLK_WRITE(0);
}

//向MOSI写入一字节，并从MISO读取一字节
uint8_t spi_read_write_byte(uint8_t byte_send)
{
	uint8_t i, byte_recv = 0x00;
	for(i = 0;i < 8;i++)
	{
		//把数据准备好在MOSI引脚
		//先发高位
		SPI_MOSI_WRITE(byte_send & (0x80 >> i));
		//上升沿采样
		SPI_SCLK_WRITE(1);
		//同时读取MISO引脚数据
		if(SPI_MISO_READ() == 1)
			byte_recv |= (0x80 >> i);
		//最后下拉时钟，完成一次采样周期
		SPI_SCLK_WRITE(0);
	}
	return byte_recv;
}

#endif
