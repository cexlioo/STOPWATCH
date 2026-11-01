#include "i2c.h"

/* 使用软件实现 I2C */
#ifdef __USE_SOFT_I2C_INTERFACE__

#define I2C_WRITE_BIT	0		/* 写控制bit */
#define I2C_READ_BIT	1		/* 读控制bit */

/* 拉低/拉高引脚 */
#define I2C_SCL_WRITE(x)		GPIO_WriteBit(I2C_SCL_PORT, I2C_SCL_PIN, (BitAction)(x))
#define I2C_SDA_WRITE(x)		GPIO_WriteBit(I2C_SDA_PORT, I2C_SDA_PIN, (BitAction)(x))

void i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(I2C_GPIO_CLK,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN;
	GPIO_Init(I2C_SCL_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
	GPIO_Init(I2C_SDA_PORT,&GPIO_InitStructure);
	
	I2C_SCL_WRITE(1);
	I2C_SDA_WRITE(1);
}

//发送起始信号
void i2c_start(void)
{
	I2C_SDA_WRITE(1);
	I2C_SCL_WRITE(1);
	os_delay(1);
	I2C_SDA_WRITE(0);
	os_delay(1);
	I2C_SCL_WRITE(0);
}

//发送停止信号
void i2c_stop(void)
{
	I2C_SDA_WRITE(0);
	I2C_SCL_WRITE(1);
	os_delay(1);
	I2C_SDA_WRITE(1);
}

//读取ACK
uint8_t i2c_read_ack(void)
{
	uint8_t sda;
	//SCL = 1 从机返回应答信号
	I2C_SCL_WRITE(1);
	os_delay(1);
	//读取SDA状态，0是ACK，1是NACK
	if(GPIO_ReadInputDataBit(I2C_SDA_PORT,I2C_SDA_PIN))
	{
		sda = 1;
	}
	else
	{
		sda = 0;
	}
	I2C_SCL_WRITE(0);
	return sda;
}

//写入一个字节
void i2c_write_byte(uint8_t byte)
{
	uint8_t i;
	for(i = 0;i < 8;i++)
	{
		//先准备数据
		if(byte & 0x80)
		{
			I2C_SDA_WRITE(1);
		}
		else
		{
			I2C_SDA_WRITE(0);
		}
		//再设时钟
		I2C_SCL_WRITE(1);
		os_delay(1);
		I2C_SCL_WRITE(0);
		os_delay(2);
		byte <<= 1;//左移一位
	}
	//等待SDA为0
	while(i2c_read_ack() != 0);
}

/**
  * @brief  发送data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_write(uint8_t slave_addr,const uint8_t* data,uint16_t data_size)
{
	i2c_start();
	//发送地址和读写位
	i2c_write_byte(slave_addr | I2C_WRITE_BIT);
	while(data_size--)
	{
		i2c_write_byte(*data++);
	}
	i2c_stop();
}

//产生ACK
void i2c_ack(void)
{
	I2C_SDA_WRITE(0);
	I2C_SCL_WRITE(1);
	os_delay(1);
	I2C_SCL_WRITE(0);
}

//不产生ACK
void i2c_nack(void)
{
	I2C_SDA_WRITE(1);
	I2C_SCL_WRITE(1);
	os_delay(1);
	I2C_SCL_WRITE(0);
}

//读取一个字节
uint8_t i2c_read_byte(void)
{
	uint8_t i, byte = 0;
	
	I2C_SDA_WRITE(1);
	for(i = 0;i < 8;i++)
	{
		I2C_SCL_WRITE(1);
		os_delay(1);
		byte <<= 1;
		if(GPIO_ReadInputDataBit(I2C_SDA_PORT,I2C_SDA_PIN))
			byte++;//byte |= 0x01;
		I2C_SCL_WRITE(0);
		os_delay(2);
	}
	return byte;
}

/**
  * @brief  读取data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_read(uint8_t slave_addr,uint8_t* data,uint16_t data_size)
{
	i2c_start();
	i2c_write_byte(slave_addr | I2C_READ_BIT);
	while(data_size--)
	{
		*data++ = i2c_read_byte();
		if(data_size == 0)
			i2c_nack();
		else
			i2c_ack();
	}
	i2c_stop();
}

/**
  * @brief  复合操作，先写入写数据，再读出一些数据，中间不需要stop信号
  * @param  slave_addr 从机地址
  * @param  write_data 写入数据的首地址
  * @param  write_size 写入数据长度
  * @param  read_data 读取数据的首地址
  * @param  read_size 读取数据长度
  * @retval 无
  */
void i2c_write_read(uint8_t slave_addr, const uint8_t* write_data,uint16_t write_size
										,uint8_t* read_data,uint16_t read_size)
{
	i2c_start();
	//写
	i2c_write_byte(slave_addr | I2C_WRITE_BIT);
	while(write_size--)
	{
		i2c_write_byte(*write_data++);
	}
	i2c_start();
	i2c_write_byte(slave_addr | I2C_READ_BIT);
	//读
	while(read_size--)
	{
		*read_data++ = i2c_read_byte();
		if(read_size == 0)
			i2c_nack();
		else
			i2c_ack();
	}
	i2c_stop();
}

#endif
