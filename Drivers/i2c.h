#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x_conf.h"
#include "system.h"

#define __USE_HARD_I2C_INTERFACE__
//#define __USE_SOFT_I2C_INTERFACE__

#define I2Cx                                I2C1
#define I2C_MASTER_ADDRESS                  0xAA
#define I2C_CLK                             RCC_APB1Periph_I2C1
#define I2C_GPIO_CLK                        RCC_APB2Periph_GPIOB
#define I2C_SCL_PORT                        GPIOB
#define I2C_SCL_PIN                         GPIO_Pin_6
#define I2C_SDA_PORT                        GPIOB 
#define I2C_SDA_PIN                         GPIO_Pin_7

/**
  * @brief  初始化I2C相关配置
  * @param  无
  * @retval 无
  */
void i2c_init(void);
/**
  * @brief  读取data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_read(uint8_t slave_addr, uint8_t* data, uint16_t data_size);
/**
  * @brief  写入data_size个字节数据
  * @param  slave_addr 从机地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void i2c_write(uint8_t slave_addr, const uint8_t* data, uint16_t data_size);
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
    uint8_t* read_data, uint16_t read_size);

#endif
