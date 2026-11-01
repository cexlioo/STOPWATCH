#include <stdio.h>
#include "i2c.h"
#include "semphr.h"

/* 使用硬件 I2C 外设 */
#ifdef __USE_HARD_I2C_INTERFACE__
static SemaphoreHandle_t i2c_mutex;

/* 初始化硬件I2C外设 */
void i2c_init(void)
{
    RCC_APB1PeriphClockCmd(I2C_CLK, ENABLE);	        /* 使能I2C1时钟 */
    RCC_APB2PeriphClockCmd(I2C_GPIO_CLK, ENABLE);       /* 使能GPIOB时钟 */

    /* STM32F103芯片的硬件I2C1: PB6 -- SCL; PB7 -- SDA */
    GPIO_InitTypeDef  GPIO_InitStructure;               /* 定义结构体配置GPIO */
    GPIO_InitStructure.GPIO_Pin =  I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		/* 设置输出模式为开漏输出，需接上拉电阻 */
    GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);       /* 初始化GPIO */
    
    I2C_DeInit(I2Cx);	/* 将外设I2C寄存器重设为缺省值 */
    I2C_InitTypeDef  I2C_InitStructure;                 /* 定义结构体配置I2C */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;			/* 工作模式 */
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;	/* 时钟占空比，Tlow/Thigh = 2 */
    I2C_InitStructure.I2C_OwnAddress1 = I2C_MASTER_ADDRESS;	/* 主机的I2C地址,用不到则随便写，无影响 */
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;	        /* 使能应答位 */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;/* 设置地址长度7位 */
    I2C_InitStructure.I2C_ClockSpeed = 400000;	        /* I2C传输速度，根据自己所用芯片手册查看支持的速度 */
    I2C_Init(I2Cx, &I2C_InitStructure);                 /* 初始化I2C */

    I2C_Cmd(I2Cx, ENABLE);  /* 启用I2C */
    
    i2c_mutex = xSemaphoreCreateMutex();
    if (!i2c_mutex) {
        printf("i2c init wrong!\n");
    }
}

/* 发送起始信号 */
void i2c_start(void)
{
    /* 发送起始信号 */
    I2C_GenerateSTART(I2Cx, ENABLE);
    /* 检测EV5事件 */
    while (!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT));
}
/* 发送停止信号 */
void i2c_stop(void)
{
    I2C_GenerateSTOP(I2Cx, ENABLE);
     /* 等待总线空闲 */
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
}
/*
 * 接收完下一个字节后，回复ACK，即高电平信号
 * 这是硬件模块自动完成的，表示期望继续接收数据
 */
void i2c_next_ack(void)
{
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}
/*
 * 接收完下一个字节后，不回复 ACK，默认是低电平信号即NACK
 * 这是硬件模块自动完成的，表示不希望继续接收数据
 * 读取最后一个字节后，回复 NACK
 */
void i2c_next_nack(void)
{
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
}
/* 
 * 设置从机地址
 * direction:
 * #define  I2C_Direction_Transmitter      ((uint8_t)0x00)  主机发到从机
 * #define  I2C_Direction_Receiver         ((uint8_t)0x01)  从机发到主机
*/
void i2c_send_slave_address(uint8_t slave_addr, uint8_t direction)
{
    /* 发送7位从机地址 + 1位读写位 */
    I2C_Send7bitAddress(I2Cx, slave_addr, direction);
    /* 检测EV6事件 */
    if(direction == I2C_Direction_Transmitter)
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    else
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}
/* 写入一个字节 */
void i2c_write_byte(uint8_t byte)
{
    /* 写入一个字节 */
    I2C_SendData(I2Cx, byte);
    /* 检测EV8_2事件 */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}
/**
  * @brief  写入data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_write(uint8_t slave_addr, const uint8_t* data, uint16_t data_size)
{
  // 保护I2C输出，防止任务切换导致数据错乱
  xSemaphoreTake(i2c_mutex, portMAX_DELAY);
  i2c_start();
  i2c_send_slave_address(slave_addr, I2C_Direction_Transmitter);
    
  while(data_size--) {
    i2c_write_byte(*data++);
	}

	i2c_stop();
  xSemaphoreGive(i2c_mutex);
}
/* 读取一个字节 */
uint8_t i2c_read_byte(void)
{
    /* 检测EV7事件 */
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	/* 读取数据并返回 */
    return I2C_ReceiveData(I2Cx);
}
/**
  * @brief  读取data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_read(uint8_t slave_addr, uint8_t* data, uint16_t data_size)
{
    xSemaphoreTake(i2c_mutex, portMAX_DELAY);

    i2c_start();
    i2c_send_slave_address(slave_addr, I2C_Direction_Receiver);
    /* 接收完下个字节，期望继续接受 */
    i2c_next_ack();
    /* 如果data_size是1，最后一个字节（减1后是0了），则设置回复 NACK */
    while(data_size--) {
		if(data_size == 0)
            i2c_next_nack();
		*data++ = i2c_read_byte();
	}

	i2c_stop();
    xSemaphoreGive(i2c_mutex);
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
void i2c_write_read(uint8_t slave_addr, const uint8_t* write_data, uint16_t write_size,
                            uint8_t* read_data, uint16_t read_size)
{
    xSemaphoreTake(i2c_mutex, portMAX_DELAY);

    i2c_start();
    i2c_send_slave_address(slave_addr, I2C_Direction_Transmitter);
    
    while(write_size--) {
        i2c_write_byte(*write_data++);
	}

    i2c_start();
    i2c_send_slave_address(slave_addr, I2C_Direction_Receiver);
    /* 接收完下个字节，期望继续接受 */
    i2c_next_ack();
    /* 如果data_size是1，最后一个字节（减1后是0了），则设置回复 NACK */
    while(read_size--) {
		if(read_size == 0)
            i2c_next_nack();
		*read_data++ = i2c_read_byte();
	}

	i2c_stop();
    xSemaphoreGive(i2c_mutex);
}
#endif
