#include "AHT10.h"

#define AHT10_ADDRESS			(0x38<<1)

//AHT10复位
void aht10_reset(void)
{
	uint8_t reset = 0xBA;
	i2c_write(AHT10_ADDRESS,&reset,1);
	//复位等待20ms
	os_delay_ms(20);
}

//读取AHT10数据 0成功 -1失败
int aht10_read(float* humi,float* temp)
{
	uint8_t temp_humi[6];//声明变量存储读取的数据
	uint8_t measure[3] = {0xAC,0x33,0x00};
	//发送指令触发测量
	i2c_write(AHT10_ADDRESS,measure,3);
	//根据手册等待75ms以上
	os_delay_ms(100);
	//读取六个字节
	i2c_read(AHT10_ADDRESS,temp_humi,6);
	
	/* 0x08(00001000)检查状态字节第 3 位(校准使能位)，如为0，则传感器未校准，不准确 */
	if((temp_humi[0] & 0x08) == 0)
	{
		aht10_reset();
		return -1;
	}
	else if((temp_humi[0] & 0x80) == 0)/* 0x80(10000000)检查状态字节第7位(忙闲指示)，0 说明测量完成 */
	{
		uint32_t SRH = (temp_humi[1]<<12) | (temp_humi[2]<<4) | (temp_humi[3]>>4);//湿度数据处理
		uint32_t ST = ((temp_humi[3]&0x0f)<<16) | (temp_humi[4]<<8) | (temp_humi[5]);//温度数据处理
		//转换数据
		*humi = (SRH * 100) / 1024.0 / 1024;
		*temp = (ST * 200) / 1024.0 / 1024 - 50;
		
		return 0;
	}
	return -1;
}
