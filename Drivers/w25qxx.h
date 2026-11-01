#ifndef __W25QXX_H
#define __W25QXX_H

#include "W25QxxInstruction.h"
#include "spi.h"
#include "stdio.h"

/**
  * @brief  初始化w25qxx，主要是初始化片选信号线
  * @param  无
  * @retval 无
  */
void w25qxx_init(void);
/**
  * @brief  读取设备的 JEDEC ID
  * @param  MID 制造商ID，Manufacturer ID，winbond固定为 0xEF
  * @param  DID 设备ID，Device ID，W25Q16 是 0x4015
  * @retval 无
  */
	
	//擦除全部
void w25qxx_chip_erase(void);
	
void w25qxx_read_id(uint8_t *MID, uint16_t *DID);
/**
  * @brief  按扇区擦除
  * @param  addr flash内部地址
  * @retval 无
  */
void w25qxx_sector_erase(uint32_t addr);
/**
  * @brief  按页写入，向地址addr写入data_size个字节数据
  * @param  addr flash内部地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void w25qxx_page_write_bytes(uint32_t addr, uint8_t* data, uint32_t data_size);
/**
  * @brief  从地址addr读取data_size个字节数据
  * @param  addr flash内部地址
  * @param  data 数据首地址
  * @param  data_size 数据长度
  * @retval 无
  */
void w25qxx_read_bytes(uint32_t addr, uint8_t* data, uint32_t data_size);
/**
  * @brief  驱动接口测试函数，用于读写测试
  * @param  无
  * @retval 无
  */
void w25qxx_test(void);

#endif
