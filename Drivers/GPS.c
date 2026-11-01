#include "GPS.h"
#include "string.h"
#include "stdio.h"
#include "bike.h"

static struct {
	char time_str[18];
	float speed_kt;	//速度，单位 节 海里每小时
	char latitude[11]; 
	char longitude[12];
	int enable;		/**< GPS是否有效定位了 */
} gps = {
	.time_str = "26-01-01 00:00:00",
	.speed_kt = 0.0,
	.latitude = "0",
	.longitude = "0",
	.enable = 0
};

int gps_enable(void)
{
    return gps.enable;
}

char* gps_get_time_str(void)
{
	return gps.time_str;
}

float gps_get_speed_kt(void)
{
	return gps.speed_kt;
}

char* gps_get_latitude(void)
{
	return gps.latitude;
}

char* gps_get_longitude(void)
{
	return gps.longitude;
}

//将UTC时间和日期转换为北京时间
static void UTC_to_BJT(const char* utc_time, const char* utc_date, char* bjt_time, char* bjt_date)
{
	int hour, minute, second, day, month, year;
	int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	sscanf(utc_time, "%2d%2d%2d", &hour, &minute, &second);
	sscanf(utc_date, "%2d%2d%2d", &day, &month, &year);
	
	//闰年二月29天
	if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
		days_in_month[1] = 29;
	
	//加八小时
	hour += 8;
	
	//小时超过24处理
	if(hour >= 24)
	{
		hour -= 24;
		day += 1;
		//日期超过当月最大日期
		if(day > days_in_month[month - 1])
		{
			day = 1;
			month += 1;
			//月份超过12
			if(month > 12)
			{
				month = 1;
				year += 1;
			}
		}
	}
	
	//格式化为北京时间
	snprintf(bjt_date, 9, "%02d-%02d-%02d", year, month, day);
	snprintf(bjt_time, 9, "%02d:%02d:%02d", hour, minute, second);
	
}

#define GNRMC_BUFFER_SIZE 128
static char gnrmc_buffer[GNRMC_BUFFER_SIZE];
static uint8_t gnrmc_received = 0;

//解析GNRMC句子
void parse_GNRMC(void)
{
	int field_index = 0, field_len = 0;
	char* start = gnrmc_buffer;
	char* end;
	
	char utc_time[11] = {0}, utc_date[7] = {0},bjt_time[9] = {0}, bjt_date[9] = {0}, speed_str[12] = {0};
	
	
	gnrmc_received = 0;//复位标志位
	//$GNRMC,040837.00,A,2237.85961,N,11314.36250,E,0.093,,310826,,,D,V*1E
	if(strncmp(gnrmc_buffer, "$GNRMC", 6) == 0)
	{
		while((end = strstr(start, ",")) != NULL)
		{
			field_len = (int)(end - start);
			//printf("field [%d]:%d", field_index, field_len);
			
			switch(field_index)
			{
				case 1://时间
					memcpy(utc_time, start, field_len);
					utc_time[sizeof(utc_time) - 1] = '\0';
					break;
				case 2://有效定位
					if (memcmp("A", start, field_len) == 0)
            gps.enable = 1;
          else if (memcmp("V", start, field_len) == 0)
            gps.enable = 0;
					break;
				case 3://纬度
					strncpy(gps.latitude, start, field_len);
					gps.latitude[sizeof(gps.latitude) - 1] = '\0';
					break;
				case 5://经度
					strncpy(gps.longitude, start, field_len);
					gps.longitude[sizeof(gps.longitude) - 1] = '\0';
					break;
				case 7://速度
					strncpy(speed_str, start, field_len);
					speed_str[sizeof(speed_str) - 1] = '\0';
					break;
				case 9://日期
					strncpy(utc_date, start, field_len);
					utc_date[sizeof(utc_date) - 1] = '\0';
					break;
				default:
					break;
			}
			
			start = end + 1;
			field_index++;
		}
		
		//printf("UTC_TIME: %s\r\n", utc_time);
		//printf("UTC_DATE: %s\r\n", utc_date);
		//转换为北京时间
		UTC_to_BJT(utc_time, utc_date, bjt_time, bjt_date);
		sscanf(speed_str, "%f", &gps.speed_kt);
		sprintf(gps.time_str, "%s %s", bjt_date, bjt_time);
		//printf("BeiJing TIME: 20%s %s\r\n",bjt_date, bjt_time);//北京时间
		//printf("latitue: %s\r\n",latitude);//纬度
		//printf("longitude: %s\r\n", longitude);//经度
		//printf("-----------------------------------\r\n");
		
	}
}

#define RX_BUFFER_SIZE 1024
static char rx_buffer[RX_BUFFER_SIZE];
static uint16_t rx_index = 0;

//GNRMC是否完整
int is_received_GNRMC(void)
{
	return gnrmc_received;
}

//$GNRMC,032055.00,A,2237.85973,N,11314.35892,E,0.067,,310826,,,D,V*18
//处理串口收到的GPS数据
void gps_data_received(uint8_t rx_data)
{
	BaseType_t taskWoken = pdFALSE;
	
	rx_buffer[rx_index++] = rx_data;
	
	//检测到换行符
	if(rx_data == '\n')
	{
		rx_buffer[rx_index] = '\0';//确保字符串以NULL结尾
		
		//比较是否是GNRMC语句
		if(strncmp(rx_buffer, "$GNRMC", 6) == 0)
		{
			strncpy(gnrmc_buffer, rx_buffer, sizeof(gnrmc_buffer) - 1);//留出'\0'的空间
			gnrmc_received = 1;//完整gnrmc语句
			
			//通知事件
			xTaskNotifyFromISR(bike_task_handle, NOTIFY_GNRMC_RECV, eSetBits, &taskWoken);
			portYIELD_FROM_ISR(taskWoken);
		}
		
		//索引清零
		rx_index = 0;
	}
	
}
