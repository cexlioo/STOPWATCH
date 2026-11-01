#include "screen.h"
#include "OLED.h"
#include "utils.h"
#include "bike.h"
#include "stdio.h"

void screen_clear(void)
{
	OLED_Clear();
}

//欢迎界面
void screen_welcome_show(char* time, int gps_enable)
{
	OLED_ShowString(1, 1, time);
	OLED_ShowString(2, 2, "welcome to use");
	OLED_ShowString(3, 3, "TU Bike!");
	if(!gps_enable)
	{
		OLED_ShowString(4, 2, "GPS NOT Ready!");
	}
	else
	{
		OLED_Clear_Part(4, 1, 16);
	}
}

//第一屏
void screen_1_show(char* time, float speed_kmh, float distance_km, 
	float total_seconds, int bike_state)
{
	/*
	1.当前时间
	2.实时速度
	3.骑行距离
	4.骑行时间
	*/
	char oled_str[32] = {0};
	char* p;
	
	OLED_ShowString(1, 1, time);
	
	sprintf(oled_str, "%.02f km/h", speed_kmh);
	OLED_ShowString(2, 1, oled_str);
	
	sprintf(oled_str, "%.02f km", distance_km);
	OLED_ShowString(3, 1, oled_str);
	
	format_seconds(total_seconds, oled_str);
	OLED_ShowString(4, 1, oled_str);
	
	p = (bike_state == RUNNING) ? "go!" : "stop";
	OLED_ShowString(4, 11, p);
}

//第二屏
void screen_2_show(float max_speed_kmh, float average_speed_kmh, 
	float kcal, float humi, float temp, int bike_state)
{
	/*
	1.最大速度
	2.平均速度
	3.卡路里
	4.实时温湿度
	*/
	char oled_str[32] = {0};
	char* p;
	
	sprintf(oled_str, "%.02f km/h", max_speed_kmh);
	OLED_ShowString(1, 1, oled_str);
	
	sprintf(oled_str, "%.02f km/h", average_speed_kmh);
	OLED_ShowString(2, 1, oled_str);
	
	sprintf(oled_str, "%.02f kcal", kcal);
	OLED_ShowString(3, 1, oled_str);
	
	sprintf(oled_str, "%.02fC %.02f%%", temp, humi);
	OLED_ShowString(4, 1, oled_str);
	
	p = (bike_state == RUNNING) ? "go!" : "stop";
	OLED_ShowString(1, 13, p);
}

void screen_init(void)
{
	OLED_Init();
}
