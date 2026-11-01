#ifndef __SCREEN_H
#define __SCREEN_H

#include "stm32f10x_conf.h"

void screen_clear(void);

//欢迎界面
void screen_welcome_show(char* time, int gps_enable);

//第一屏
void screen_1_show(char* time, float speed_kmh, float distance_km, 
	float total_seconds, int bike_state);

//第二屏
void screen_2_show(float max_speed_kmh, float average_speed_kmh, 
	float kcal, float humi, float temp, int bike_state);

void screen_init(void);

#endif
