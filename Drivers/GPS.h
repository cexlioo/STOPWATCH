#ifndef __GPS_H
#define __GPS_H

#include "stm32f10x_conf.h"

//GNRMC是否完整
int is_received_GNRMC(void);
//解析GNRMC句子
void parse_GNRMC(void);
//处理串口收到的GPS数据
void gps_data_received(uint8_t rx_data);

int gps_enable(void);

char* gps_get_time_str(void);
float gps_get_speed_kt(void);
char* gps_get_latitude(void);
char* gps_get_longitude(void);

#endif
