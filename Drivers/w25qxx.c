#include "w25qxx.h"

#define W25QXX_NSS_PORT				GPIOA
#define W25QXX_NSS_CLK				RCC_APB2Periph_GPIOA
#define W25QXX_NSS_PIN				GPIO_Pin_4

#define W25QXX_NSS_HIGH()			GPIO_WriteBit(W25QXX_NSS_PORT, W25QXX_NSS_PIN, Bit_SET)
#define W25QXX_NSS_LOW()			GPIO_WriteBit(W25QXX_NSS_PORT, W25QXX_NSS_PIN, Bit_RESET)

void w25qxx_init(void)
{
	RCC_APB2PeriphClockCmd(W25QXX_NSS_CLK,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = W25QXX_NSS_PIN;
	GPIO_Init(W25QXX_NSS_PORT,&GPIO_InitStructure);
	
	W25QXX_NSS_HIGH();//拉高空闲
}

//读取JEDEC ID
void w25qxx_read_id(uint8_t* MID,uint16_t* DID)
{
	W25QXX_NSS_LOW();//拉低，开始通信
	//发送命令
	spi_read_write_byte(W25Qxx_JEDEC_ID);
	
	*MID = spi_read_write_byte(W25Qxx_DUMMY_BYTE);
	*DID = spi_read_write_byte(W25Qxx_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= spi_read_write_byte(W25Qxx_DUMMY_BYTE);
	
	W25QXX_NSS_HIGH();//拉高，通信结束
}

//写使能
static void w25qxx_write_enable(void)
{
	W25QXX_NSS_LOW();
	spi_read_write_byte(W25Qxx_WRITE_ENABLE);
	W25QXX_NSS_HIGH();
}

//读取芯片状态，如果忙则等待，一直等到其空闲，或者100微秒超时
static void w25qxx_wait_for_idel(void)
{
	uint8_t retry = 0;
	W25QXX_NSS_LOW();
	spi_read_write_byte(W25Qxx_READ_STATUS_REGISTER_1);
	//等待一号寄存器置位
	while((spi_read_write_byte(W25Qxx_DUMMY_BYTE) & W25Qxx_REGISTER_1_BUSY_BIT) && retry < 100)
	{
		retry++;
		os_delay(1);
	}
	W25QXX_NSS_HIGH();
}

//按扇区擦除
void w25qxx_sector_erase(uint32_t addr)
{
	w25qxx_write_enable();
	W25QXX_NSS_LOW();
	//命令
	spi_read_write_byte(W25Qxx_SECTOR_ERASE_4KB);
	spi_read_write_byte(addr >> 16);
	spi_read_write_byte(addr >> 8);
	spi_read_write_byte(addr);
	W25QXX_NSS_HIGH();
	//等待一号寄存器置位
	w25qxx_wait_for_idel();
	os_delay(1000); /* 擦除后给芯片一点反应时间 */
}

//擦除全部
void w25qxx_chip_erase(void)
{
	w25qxx_write_enable();
	
	W25QXX_NSS_LOW();
	spi_read_write_byte(W25Qxx_CHIP_ERASE);
	
	W25QXX_NSS_HIGH();
	
	w25qxx_wait_for_idel();
	os_delay(1000);
}

//写入数据,按页
void w25qxx_page_write_bytes(uint32_t addr, uint8_t* data, uint32_t data_size)
{
	//写使能
	w25qxx_write_enable();
	W25QXX_NSS_LOW();
	spi_read_write_byte(W25Qxx_PAGE_PROGRAM);
	//写地址
	spi_read_write_byte(addr >> 16);
	spi_read_write_byte(addr >> 8);
	spi_read_write_byte(addr);
	while(data_size--)
	{
		spi_read_write_byte(*data++);
	}
	W25QXX_NSS_HIGH();
	//等待置位
	w25qxx_wait_for_idel();
	os_delay(1000);
}

//读取数据
void w25qxx_read_bytes(uint32_t addr, uint8_t* data, uint32_t data_size)
{
	W25QXX_NSS_LOW();
	spi_read_write_byte(W25Qxx_READ_DATA);
	spi_read_write_byte(addr >> 16);
	spi_read_write_byte(addr >> 8);
	spi_read_write_byte(addr);
	while(data_size--)
	{
		*data++ = spi_read_write_byte(W25Qxx_DUMMY_BYTE);
	}
	W25QXX_NSS_HIGH();
}

/* 打印十六进制数组 */
void print_hex_array(const char* prefix, uint8_t* array, int size) 
{
    int i;
    printf("%s\r\n", prefix);
    for (i = 0; i < size; i++) {
        printf("0x%02X ", array[i]);
        if(i % 16 == 15) {
            printf("\r\n");
        }
    }
}

void w25qxx_test(void)
{
	#define W25QXX_TEST_LEN 256
	uint8_t write_buf[W25QXX_TEST_LEN];
	uint8_t read_buf[W25QXX_TEST_LEN];
	int i;
	uint8_t MID;
	uint16_t DID;
	
	printf("SPI Flash W25QXX Test\r\n");
	//初始化
	w25qxx_init();
	//获取设备MID DID
	w25qxx_read_id(&MID,&DID);
	printf("MID:0x%X, DID:0x%X\r\n", MID, DID);
	//校验ID
	if(MID != 0xEF)
	{
		printf("SPI Flash W25QXX Test fail\r\n");
		return;
	}
	//擦除4K内容
	w25qxx_sector_erase(0x00);
	os_delay_ms(1);//给芯片一点反应时间
	for(i = 0;i < W25QXX_TEST_LEN;i++)
	{
		write_buf[i] = i;
	}
	print_hex_array("============[write data]============",write_buf,W25QXX_TEST_LEN);
	w25qxx_page_write_bytes(0x00,write_buf,W25QXX_TEST_LEN);
	os_delay_ms(1);//给芯片一点反应时间
	w25qxx_read_bytes(0x00,read_buf,W25QXX_TEST_LEN);
	print_hex_array("============[read data]============",read_buf,W25QXX_TEST_LEN);
	
	//对比
	for(i = 0;i < W25QXX_TEST_LEN;i++)
	{
		if(read_buf[i] != write_buf[i])
		{
			printf("0x%02X ",read_buf[i]);
			printf("SPI Flash W25QXX test fail\r\n");
			return;
		}
	}
	printf("SPI Flash W25QXX test success\r\n");
}
