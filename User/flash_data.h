#ifndef __FLASH_DATA_H
#define __FLASH_DATA_H

#include "stm32f10x_conf.h"
#include "bike.h"

//骑行完毕应当将write_first_item 置1，为下一次骑行做准备
void set_first_write_flag(void);

//输出一次骑行数据
void print_table_n(int n);

void save_item(char* lat, char* lon, float speed_kmh, enum BIKE_STATE bike_state);

void init_flash(void);


#endif
