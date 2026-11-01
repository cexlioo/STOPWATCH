#ifndef __UTILS_H
#define __UTILS_H

#include "stm32f10x_conf.h"

//无符号整数转成缩小一百倍的浮点数
float uint16_to_float(uint16_t value);

//浮点数转成放大100倍的无符号整数
uint16_t float_to_uint16(float value);

//有符号整数转成缩小一百倍的浮点数
float int16_to_float(int16_t value);

//浮点数转成放大100倍的有符号整数
int16_t float_to_int16(float value);

//时间戳转换成时间字符串
char* timestamp_to_time_str(uint32_t t, char* time_str);

//时间字符串转换为时间戳
uint32_t time_str_to_timestamp(char* time_str);

//将秒数转换成格式 hh:mm:ss
char* format_seconds(int total_seconds, char* formatted_time);

/* 转换纬度 */
double convert_lat(double x, double y);

/* 转换经度 */
double convert_lon(double x, double y);

/* WGS-84坐标转换为GCJ-02坐标 */
void gps_to_gcj02(double wgs_lat, double wgs_lon, double* gcj_lat, double* gcj_lon);

//将度、分形式改为十进制的度形式
double convert_to_decimal_degrees(double degree_minutes);

//计算平均温度
void calc_average_humi_temp(float humi, float temp, float* average_humi,
	float* average_temp, int reset_data);

//计算sec秒的骑行距离
float calc_distance_m(float speed_kt, uint32_t sec);

//骑行总距离
float calc_total_distance_km(float distance_km, int reset_data);

//计算消耗卡路里
float calc_kcal(float total_distance_km);

//计算实时速度
float calc_speed_kmh(float speed_kt);

//计算最大速度
float calc_max_speed_kmh(float speed_kmh, int reset_data);

//计算平均时速
float calc_average_speed(float total_distance_km, uint32_t total_time_sec);

#endif
