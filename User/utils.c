#include "utils.h"
#include "stdio.h"
#include "math.h"
#include "time.h"

#define WEIGHT_KG 70			//体重，单位kg,用于计算卡路里
#define WEIGHT_FACTOR 0.5	//体重系数，用于计算卡路里

//无符号整数转成缩小一百倍的浮点数
float uint16_to_float(uint16_t value)
{
	//缩小100倍
	float result = value / 100.0f;
	return result;
}

//浮点数转成放大100倍的无符号整数
uint16_t float_to_uint16(float value)
{
	uint16_t result;
	float scaled_value = value * 100.0f;
	
	//四舍五入,不考虑负数
	if(scaled_value > 0)
	{
		result = (uint16_t)(scaled_value + 0.5f);
	}
	else
	{
		result = 0;
	}
	return result;
}

//有符号整数转成缩小一百倍的浮点数
float int16_to_float(int16_t value)
{
	float result = value / 100.0f;
	return result;
}

//浮点数转成放大100倍的有符号整数
int16_t float_to_int16(float value)
{
	int16_t result;
	float scaled_value = value * 100.0f;
	
	//四舍五入
	if(scaled_value > 0)
	{
		result = (int16_t)(scaled_value + 0.5f);
	}
	else
	{
		result = (int16_t)(scaled_value - 0.5f);
	}
	return result;
}

//将秒数转换成格式 hh:mm:ss
char* format_seconds(int total_seconds, char* formatted_time)
{
	int hours, minutes, seconds;
	
	hours = total_seconds / 3600;
	total_seconds %= 3600;
	minutes = total_seconds / 60;
	seconds = total_seconds % 60;
	
	sprintf(formatted_time, "%02d:%02d:%02d", hours, minutes, seconds);
	return formatted_time;
}

//时间戳转换成时间字符串
char* timestamp_to_time_str(uint32_t t, char* time_str)
{
	int year, month, day, hour, minute, second;
	time_t days, seconds;
	
	//计算天数和秒数
	days = t / 86400;
	seconds = t % 86400;
	
	//计算小时分钟和秒
	hour = seconds / 3600;
	seconds %= 3600;
	minute = seconds / 60;
	second = seconds % 60;
	
	//从1970年1月1日开始计算年
	year = 1970;
	while(1)
	{
		int days_in_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 366 : 365;
		if(days >= days_in_year)
		{
			days -= days_in_year;
			year++;
		}
		else
		{
			break;
		}
	}
	
	//剩下的days算月份
	month = 0;
	static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	static const int days_in_month_leap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	const int *days_per_month = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? days_in_month_leap : days_in_month;
	
	for(month = 0; month < 12; month++)
	{
		if(days >= days_per_month[month])
		{
			days -= days_per_month[month];
		}
		else
		{
			break;
		}
	}
	day = days + 1;
	
	// 年份减去1900，月份加1（因为tm结构中的月份从0开始）
	year -= 1900;
	month += 1;
	
	//初始化字符串
	sprintf(time_str, "%02d-%02d-%02d %02d:%02d:%02d", year % 100, month, day, hour, minute, second);
	
	return time_str;
}

//时间字符串转换为时间戳
uint32_t time_str_to_timestamp(char* time_str)
{
	struct tm tm;
	
	//初始化结构体tm
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = 0;
	tm.tm_mon = 0;
	tm.tm_year = 0;
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = -1;		// 让mktime自动判断是否夏令时
	
	//解析字符串
	sscanf(time_str, "%2d-%2d-%2d %2d:%2d:%2d", 
				&tm.tm_year, &tm.tm_mon, &tm.tm_mday,
				&tm.tm_hour, &tm.tm_min, &tm.tm_sec);
	
	// 年份是从1900年开始的，所以需要加上1900
  tm.tm_year += 100; // 从26到2026年，因此加上100
	// 月份是从0开始的，所以需要减去1
	tm.tm_mon -= 1;
	
	return (uint32_t)mktime(&tm);
}

#define pi 3.1415926535897932384626
#define a 6378245.0
#define ee 0.00669342162296594323
/* 转换纬度 */
double convert_lat(double x, double y)
{
  double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
  ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
  ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
  return ret;
}

/* 转换经度 */
double convert_lon(double x, double y)
{
  double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
  ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
  ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
  ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
  return ret;
}

/* WGS-84坐标转换为GCJ-02坐标 */
void gps_to_gcj02(double wgs_lat, double wgs_lon, double* gcj_lat, double* gcj_lon)
{
  double dLat = convert_lat(wgs_lon - 105.0, wgs_lat - 35.0);
  double dLon = convert_lon(wgs_lon - 105.0, wgs_lat - 35.0);
  double radLat = wgs_lat / 180.0 * pi;
  double magic = sin(radLat);
  magic = 1 - ee * magic * magic;
  double sqrtMagic = sqrt(magic);
  dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
  dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
  *gcj_lat = wgs_lat + dLat;
  *gcj_lon = wgs_lon + dLon;
}

//将度、分形式改为十进制的度形式
double convert_to_decimal_degrees(double degree_minutes)
{
	int degrees = (int)(degree_minutes / 100);//提取度
	double minutes = degree_minutes - (degrees * 100);//提取分
	return degrees + (minutes / 60.0);//计算十进制
}

//计算平均温度
void calc_average_humi_temp(float humi, float temp, float* average_humi, float* average_temp, int reset_data)
{
	static float total_humi = 0.0;
	static float total_temp = 0.0;
	static uint32_t cnt = 0;
	
	//判断是否需要重置累计值
	if(reset_data)
	{
		total_humi = 0.0;
		total_temp = 0.0;
		cnt = 0;
	}
	
	total_humi += humi;
	total_temp += temp;
	
	cnt++;
	
	*average_humi = total_humi / cnt;
	*average_temp = total_temp / cnt;
}

//计算sec秒的骑行距离
float calc_distance_m(float speed_kt, uint32_t sec)
{
	return speed_kt * 0.514 * sec;
}

//骑行总距离
float calc_total_distance_km(float distance_km, int reset_data)
{
	static float total_distance_km = 0.0;
	
	if(reset_data)
	{
		total_distance_km = 0.0;
	}
	
	total_distance_km += distance_km;
	
	return total_distance_km;
}

//计算消耗卡路里
float calc_kcal(float total_distance_km)
{
	return WEIGHT_KG * WEIGHT_FACTOR * total_distance_km;
}

//计算实时速度
float calc_speed_kmh(float speed_kt)
{
	return speed_kt * 1.852;
}

//计算最大速度
float calc_max_speed_kmh(float speed_kmh, int reset_data)
{
	static float max_speed_kmh = 0.0;
	
	if(reset_data)
	{
		max_speed_kmh = 0.0;
	}
	
	if(speed_kmh > max_speed_kmh)
		max_speed_kmh = speed_kmh;
	
	return max_speed_kmh;
}

//计算平均时速
float calc_average_speed(float total_distance_km, uint32_t total_time_sec)
{
	return total_distance_km * 1000 / (float)total_time_sec * 3.6;
}
