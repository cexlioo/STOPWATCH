#include "spi.h"

#ifdef __USE_HARD_SPI_INTERFACE__

void spi_init(void)
{
	//使能引脚和SPI
	RCC_APB2PeriphClockCmd(SPI_CLK,ENABLE);
	
	//配置结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	//SCLK MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI_CLK_PIN | SPI_MOSI_PIN;		//时钟和输出引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_CLK_PORT,&GPIO_InitStructure);
	//MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;				//上拉输入
	GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;				//输入引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_MISO_PORT,&GPIO_InitStructure);
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//主模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//全双工模式
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;			//spi空闲时，scl处于低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		//数据在sclk奇数边沿采样
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	//对时钟分频
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//每次通信数据大小为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//从高有效位开始传输
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;					//使用软件控制NSS线，而不是硬件
	SPI_InitStructure.SPI_CRCPolynomial = 0;				/* CRC多项式校验，大多数从机不需要校验，这个变量无效 */
	SPI_Init(SPIx,&SPI_InitStructure);
	
	SPI_Cmd(SPIx,ENABLE);

}

//向MOSI写入一字节，并从MISO读取一字节
uint8_t spi_read_write_byte(uint8_t byte_send)
{
	while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE) != SET);//等待写入置位
	SPI_I2S_SendData(SPIx,byte_send);//发送数据
	while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE) != SET);//等待有数据可读
	return SPI_I2S_ReceiveData(SPIx);
}

#endif
