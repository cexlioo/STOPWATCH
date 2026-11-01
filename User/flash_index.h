#ifndef __FLASH_INDEX_H
#define __FLASH_INDEX_H

#include "stm32f10x_conf.h"

/*< flash页写入标记 */
#define FLASH_WRITE_FLAG 0xA5

void write_index(void);

void set_table_created_timestamp(uint32_t ts);

//n: 1~16
uint32_t get_table_n_created_timestamp(int n);

//n: 1~16
uint32_t get_table_n_addr(int n);

uint32_t get_next_table_addr(void);

uint16_t get_table_cnt(void);

void init_index(void);

#endif
