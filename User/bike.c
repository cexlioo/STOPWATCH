#include "AHT10.h"
#include "GPS.h"
#include "string.h"
#include "stdio.h"
#include "usart.h"
#include "bike.h"
#include "utils.h"
#include "screen.h"
#include "key.h"
#include "flash_index.h"
#include "flash_data.h"
#include "cJSON.h"
#include "at_M100p.h"
#include "stdlib.h"

#define TCP_SERVER_IP "8.138.19.16"
#define TCP_SERVER_PORT 3389

#define ITEM_INTERVAL_S                 10              /**< 写入记录的时间间隔 */
#define UPLOAD_INTERVAL_S               2               /**< 上传到服务器的时间间隔 */
#define REQ_TIME_INTERVAL_S     				180 						/**< 请求时间同步的时间间隔 */

uint32_t timestamp_local = 0;//本地时间戳

TaskHandle_t bike_task_handle = NULL;
TimerHandle_t timer_handle_bike = NULL;

/*
屏幕状态
0：欢迎界面
1：显示第一屏信息
2：显示第二屏信息
*/
enum SCREEN_NO{
	SCREEN_WELCOME,
	SCREEN_1,
	SCREEN_2,
};

struct bike
{
	float total_distance_km;		//骑行距离，单位 千米
	uint32_t total_time_sec;		//骑行时间，单位 秒
	float speed_kmh;					//实时速度，单位 千米每小时
	float max_speed_kmh;			//最大速度，单位 千米每小时
	enum BIKE_STATE bike_state;       /**< 码表状态，按键触发切换顺序：0--->1--->2--->0 */
  enum RUNNING_STATE running_state; /**< 运行状态，联网触发切换顺序 */
	enum SCREEN_NO current_screen;
	
	float								average_speed_kmh;						//平均速度
	float 							humi;													//实时湿度
	float 							temp;													//实时温度
	float 							average_humi;									//平均湿度
	float 							average_temp;									//平均温度
	float 							kcal;													//消耗卡路里
	uint8_t							reset_data;										//是否重置累计变量的标记位
	
	uint8_t							data_export_n;								//导出数据项编号
};

static struct bike my_bike;

const char bike_name[32] = "dev_bike_kieran";		//设备名称
const char bike_type[8] = "bike";								//设备类型

void m100p_recv_data_cb(const char* data, int len);

void bike_reset(void);

float bike_get_distance_km(void)
{
	return my_bike.total_distance_km;
}

char* bike_get_time_str(void)
{
	return gps_get_time_str();
}

float bike_get_max_speed_kmh(void)
{
	return my_bike.max_speed_kmh;
}

float bike_get_average_speed_kmh(void)
{
	return my_bike.average_speed_kmh;
}

float bike_get_kcal(void)
{
	return my_bike.kcal;
}

float bike_get_average_humi(void)
{
	return my_bike.average_humi;
}

float bike_get_average_temp(void)
{
	return my_bike.average_temp;
}

static void bike_screen_show(void)
{
	char time_str[18] = {0};
	strcpy(time_str, gps_get_time_str());
	
	if(my_bike.current_screen == SCREEN_WELCOME)
	{
		screen_welcome_show(&time_str[3], gps_enable());
	}
	else if(my_bike.current_screen == SCREEN_1)
	{
		screen_1_show(&time_str[3], my_bike.speed_kmh, my_bike.total_distance_km,
		 my_bike.total_time_sec, my_bike.bike_state);
	}
	else if(my_bike.current_screen == SCREEN_2)
	{
		screen_2_show(my_bike.max_speed_kmh, my_bike.average_speed_kmh, my_bike.kcal, 
		 my_bike.humi, my_bike.temp, my_bike.bike_state);
	}
}

void bike_screen_change(void)
{
	if(my_bike.current_screen == SCREEN_1)
	{
		my_bike.current_screen = SCREEN_2;
		screen_clear();
	}
	else if(my_bike.current_screen == SCREEN_2)
	{
		my_bike.current_screen = SCREEN_1;
		screen_clear();
	}
}

int parse_json_time(const char* json_str, uint32_t* time_out)
{
	if(!json_str || !time_out)
	{
		return 0;//参数无效
	}
	
	cJSON *json = cJSON_Parse(json_str);
	if(!json)
	{
		return 0;//解析失败
	}
	
	//尝试获取time字段
	cJSON *time = cJSON_GetObjectItem(json, "time");
	if(!time || !cJSON_IsNumber(time))
	{
		cJSON_Delete(json);
		return 0;//没有time字段或字段不是数字
	}
	
	*time_out = (uint32_t)time->valuedouble;
	
	cJSON_Delete(json);
	
	return 1;//成功
}

//判断字符串是否合法json，并解析cmd
int parse_json_cmd(const char* json_str, char* cmd_out, size_t cmd_out_size)
{
	//检验参数合法性
	if(!json_str || !cmd_out || cmd_out_size == 0)
	{
		return 0;
	}
	
	cJSON* json = cJSON_Parse(json_str);
	if(!json)
	{
		return 0;
	}
	
	cJSON* cmd = cJSON_GetObjectItem(json, "cmd");
	if(!cmd || !cJSON_IsString(cmd))
	{
		cJSON_Delete(json);
		return 0;
	}
	
	strncpy(cmd_out, cmd->valuestring, cmd_out_size - 1);
	cmd_out[cmd_out_size - 1] = '\0';
	
	//释放json对象
	cJSON_Delete(json);
	
	return 1;
}

int bike_handle_server_data(const char* json_str)
{
	char cmd[32] = {0};	//用于存储解析到的cmd值
	
	if(parse_json_cmd(json_str, cmd, sizeof(cmd)))
	{
		Log_i("cmd [%s] from json: %s\n", cmd, json_str);
		if(strcmp(cmd, "time response") == 0)
		{
			uint32_t timestamp_cloud;
			if(parse_json_time(json_str, &timestamp_cloud))
			{
				timestamp_local = timestamp_cloud + (8 * 3600);//更新本地时间戳
				xTaskNotify(bike_task_handle, NOTIFY_TIME_UPDATE, eSetBits);
			}
		}
		else
		{
			Log_w("cmd not support\n");
		}
		return 0;
	}
	else
	{
		return -1;//没有收到一条完整的json
	}
}

//向服务器请求时间
void bike_cloud_time_request(void)
{
	char* json_str = "{\"cmd\":\"time request\"}";
	Log_i("***************time request***************\n");
	m100p_send_data(json_str, strlen(json_str));
}

void get_gcj(char* lat, char* lon, double* gcj_lat, double* gcj_lon)
{
	double lat_deg;	//原始数据
	double lon_deg;	
	double lat_dec;	//十进制
	double lon_dec;
	
	sscanf(lat, "%lf", &lat_deg);
	sscanf(lon, "%lf", &lon_deg);
	
	lat_dec = convert_to_decimal_degrees(lat_deg);
	lon_dec = convert_to_decimal_degrees(lon_deg);
	
	gps_to_gcj02(lat_dec, lon_dec, gcj_lat, gcj_lon);
	
	Log_d("get_gcj lat: %.16lf, lng: %.16lf\n", *gcj_lat, *gcj_lon);
}

//添加带浮点精度限制的数字到cjson对象中
void cJSON_AddNumberWithPrecision(cJSON* obj, const char* key, double value, int precision)
{
	char buffer[32];//足够存储格式化的浮点数
	snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
	cJSON_AddItemToObject(obj, key, cJSON_CreateRaw(buffer));
}

//将bike数据结构组装成json字符串
char* encode_bike_upload(void)
{
	double gcj_lat, gcj_lon;
	//创建json根对象
	cJSON* json_root = cJSON_CreateObject();
	if(json_root == NULL)
	{
		return NULL;
	}
	
	//添加基础字段
	cJSON_AddStringToObject(json_root, "cmd", "upload data");
	cJSON_AddStringToObject(json_root, "name", bike_name);
	cJSON_AddStringToObject(json_root, "type", bike_type);
	cJSON_AddNumberToObject(json_root, "time", timestamp_local - (8 * 3600));
	
	//创建json子对象
	cJSON* json_data = cJSON_CreateObject();
	if(json_data == NULL)
	{
		cJSON_Delete(json_root);
		return NULL;
	}
	
	// 添加数据到 data 子对象，控制浮点精度为 2 位
  cJSON_AddNumberWithPrecision(json_data, "temp", my_bike.temp, 2);
  cJSON_AddNumberWithPrecision(json_data, "humidity", my_bike.humi, 2);
  cJSON_AddNumberWithPrecision(json_data, "speed", my_bike.speed_kmh, 2);
  cJSON_AddNumberWithPrecision(json_data, "distance", my_bike.total_distance_km, 2);
  cJSON_AddNumberToObject(json_data, "total_time", my_bike.total_time_sec);
  cJSON_AddNumberWithPrecision(json_data, "max_speed", my_bike.max_speed_kmh, 2);
  cJSON_AddNumberWithPrecision(json_data, "average_speed", my_bike.average_speed_kmh, 2);
  cJSON_AddNumberWithPrecision(json_data, "kcal", my_bike.kcal, 2);

  get_gcj(gps_get_latitude(), gps_get_longitude(), &gcj_lat, &gcj_lon);
  cJSON_AddNumberWithPrecision(json_data, "lat", gcj_lat, 6);
  cJSON_AddNumberWithPrecision(json_data, "lng", gcj_lon, 6);
	
	//将data子对象添加到根对象
	cJSON_AddItemToObject(json_root, "data", json_data);
	
	//转换成json字符串
	char *json_string = cJSON_PrintUnformatted(json_root);
	//printf("%s\n", json_string);
	
	//释放json对象
	cJSON_Delete(json_root);
	
	return json_string;
}

//将json发送到服务器
void bike_cloud_upload_data(void)
{
	//组装json
	char* json_str = encode_bike_upload();
	if(json_str != NULL)
	{
		Log_i("***************upload data***************\n");
		
		m100p_send_data(json_str, strlen(json_str));
		os_free(json_str);//释放内存
	}
	else
	{
		Log_e("failed to generate JSON.\n");
	}
}

void bike_running_state_change(void)
{
	//Log_i("bike state now :%d\n", my_bike.bike_state);
	
	if(my_bike.bike_state == IDEL && gps_enable())	//在GPS有效前不响应按钮
	{
		screen_clear();
		my_bike.bike_state = RUNNING;
		my_bike.current_screen = SCREEN_1;
		set_table_created_timestamp(time_str_to_timestamp(gps_get_time_str()));
		//配置网络，激活pdp上下文，连接服务器
		m100p_start(TCP_SERVER_IP, TCP_SERVER_PORT, m100p_recv_data_cb);
	}
	else if(my_bike.bike_state == RUNNING)
	{
		my_bike.bike_state = STOP;
		m100p_stop();
		//将本次骑行未写入flash的item写入
		save_item(gps_get_latitude(), gps_get_longitude(), my_bike.speed_kmh, my_bike.bike_state);
	}
	else if(my_bike.bike_state == STOP)
	{
		screen_clear();
		bike_reset();
	}
	Log_i("bike state change:%d\n", my_bike.bike_state);
}

int is_key1_press(void)
{
	//切换骑行状态按键
	if(GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0)
	{
		os_delay_ms(20);//消抖
		if(GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0)
		{
			while(GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN) == 0);
			os_delay_ms(20);
			return 1;
		}
	}
	return 0;
}

int is_key2_press(void)
{
	//界面切换
	if(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0)
	{
		os_delay_ms(20);
		if(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0)
		{
			//等待按键松开
			while(GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) == 0);
			os_delay_ms(20);
			return 1;
		}
	}
	return 0;
}

//运行后一秒调用一次
static void bike_calc_running_data(void)
{
	float speed_kt = gps_get_speed_kt();
	
	float distance_km = calc_distance_m(speed_kt, 1) / 1000;
	
	my_bike.speed_kmh = calc_speed_kmh(speed_kt);
	my_bike.total_distance_km = calc_total_distance_km(distance_km, my_bike.reset_data);
	my_bike.max_speed_kmh = calc_max_speed_kmh(my_bike.speed_kmh, my_bike.reset_data);
	my_bike.average_speed_kmh = calc_average_speed(my_bike.total_distance_km, my_bike.total_time_sec);
	my_bike.kcal = calc_kcal(my_bike.total_distance_km);
	
	//读取当前温湿度
	aht10_read(&my_bike.humi, &my_bike.temp);
	calc_average_humi_temp(my_bike.humi, my_bike.temp, 
		&my_bike.average_humi, &my_bike.average_temp, my_bike.reset_data);
	
	if(my_bike.reset_data)
		my_bike.reset_data = 0;
}

//打印可查询的骑行数据列表
static void bike_data_do_list(void)
{
	int i; 
	char time_str[32] = {0};
	int cnt = get_table_cnt();
	
	os_printf("===============================================\n");
	if(cnt > 0)
	{
		os_printf("There are [%d] riding data in flash, select number to export [1-%d]\n", cnt, cnt);
		for(i = 1; i <= cnt; i++)
		{
			os_printf("riding [%d], created at %s\n", 
				i, timestamp_to_time_str(get_table_n_created_timestamp(i), time_str));
		}
	}
	else
	{
		os_printf("There is [0] riding data in flash\n");
	}
	os_printf("===============================================\n");
}

//自行车数据导出
static int bike_data_export(void)
{
	if(my_bike.data_export_n != 0)
	{
		if(my_bike.bike_state != IDEL)//空闲才能导出数据
		{
			my_bike.data_export_n = 0;
			Log_w("data export only in IDEL status!\n");
			return -1;
		}
		
		print_table_n(my_bike.data_export_n);
		my_bike.data_export_n = 0;
	}
	
	return 0;
}

//自行车数据导出
void bike_data_export_request(char* cmd)
{
	int n;
	
	if(sscanf(cmd, "%d", &n) == 1)
	{
		if(n >= 1 && n <= 16)
		{
			my_bike.data_export_n = n;
		}
		else
		{
			//printf("invalid cmd [%d]\n", n);
		}
	}
}

void bike_reset(void)
{
	memset(&my_bike, 0, sizeof(struct bike));
	

	my_bike.total_distance_km = 0.0;
	my_bike.total_time_sec = 0;
	my_bike.speed_kmh = 0.0;
	my_bike.max_speed_kmh = 0.0;

	my_bike.bike_state = IDEL;
	my_bike.running_state = RUNNING_NOT_CONNECTED_SERVER;
	my_bike.current_screen = SCREEN_WELCOME;
	
	my_bike.reset_data = 1;
	set_first_write_flag();
}

void bike_init(void)
{
	memset(&my_bike, 0, sizeof(struct bike));
	
	key_init();
	i2c_init();
	screen_init();
	bike_reset();
	
	init_flash();
	
	m100p_init();
}

//FreeRTOS新增内容

//收到4G数据的回调函数
void m100p_recv_data_cb(const char* data, int len)
{
	#define RECV_BUF_SIZE 512
	static char recv_buf[RECV_BUF_SIZE];  // 缓冲区
  static int total_len = 0;             // 当前累计的数据长度
	
	if(len <= 0)
	{
		return;//无数据
	}
	
	//检查缓冲区是否可以装下这次收到的数据
	if(total_len + len >= RECV_BUF_SIZE - 1)
	{
		Log_e("Receive buffer overflow, clearing buffer\n");
		total_len = 0;
		return;
	}
	
	//把收到的数据拷贝到缓冲区
	memcpy(recv_buf + total_len, data, len);
	total_len += len;
	recv_buf[total_len] = '\0';
	
	//处理数据
	if(bike_handle_server_data(recv_buf) == 0)
	{
		// 处理成功，清空缓冲区
		total_len = 0;
	}
	else
	{
		// 处理失败，继续等待后续数据
    Log_e("Invalid JSON:[%d], %s, waiting for more data...\n", total_len, recv_buf);
	}
}

void bike_task(void *pvParameters)
{
	uint32_t notify_bits = 0;
	
	while(1)
	{
		xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t*)&notify_bits, portMAX_DELAY);
		
		if(notify_bits & NOTIFY_KEY1_PRESS)
		{
			Log_d("NOTIFY_KEY1_PRESS\n");
			if(is_key1_press())
				bike_running_state_change();
		}
		
		if(notify_bits & NOTIFY_KEY2_PRESS)
		{
			Log_d("NOTIFY_KEY2_PRESS\n");
			if(is_key2_press())
				bike_screen_change();
		}
		
		if(notify_bits & NOTIFY_GNRMC_RECV)//接收到gps数据
		{
			//Log_d("NOTIFY_GNRMC_RECV\n");
			parse_GNRMC();
		}
		
		if(notify_bits & NOTIFY_CONNECTED_SERVER)
		{
			Log_d("NOTIFY_CONNECTED_SERVER\n");
			my_bike.running_state = RUNNING_CONNECTED_SERVER;
		}
		
		if(notify_bits & NOTIFY_TIME_UPDATE)//时间同步成功
		{
			Log_d("NOTIFY_TIME_UPDATE\n");
			my_bike.running_state = RUNNING_UPDATED_TIME;
		}
		
		if(notify_bits & NOTIFY_DATA_LIST)//列出数据
		{
			Log_d("NOTIFY_DATA_LIST\n");
			bike_data_do_list();
		}
		
		if(notify_bits & NOTIFY_DATA_EXPORT)//导出数据
		{
			Log_d("NOTIFY_DATA_EXPORT\n");
			bike_data_export();
		}
	}
}

void bike_local_handle(void)
{
    my_bike.total_time_sec++;   // 累加骑行总时长

    bike_calc_running_data();

    /* 每隔10秒，保存一条经纬速度 */
    if (my_bike.total_time_sec % ITEM_INTERVAL_S == 0)
    {
        save_item(gps_get_latitude(), gps_get_longitude(), my_bike.speed_kmh, my_bike.bike_state);
    }
}

void bike_online_handle(void)
{
	switch(my_bike.running_state)
	{
		case RUNNING_UPDATED_TIME:	//和服务器同步时间后
			timestamp_local++;
			if(timestamp_local % UPLOAD_INTERVAL_S == 0)//每两秒上传数据
				bike_cloud_upload_data();
			if(timestamp_local % REQ_TIME_INTERVAL_S == 0)//每三分钟请求时间
				bike_cloud_time_request();
		break;
		case RUNNING_CONNECTED_SERVER:	//连接服务器后还没有同步时间，每隔一秒同步时间
			bike_cloud_time_request();
			break;
		case RUNNING_NOT_CONNECTED_SERVER:
			break;
		default:break;
	}
}

void timer_handle_bike_cb(TimerHandle_t xTimer)
{
	//每秒更新屏幕
	bike_screen_show();
	
	if(my_bike.bike_state == RUNNING)
	{
		bike_local_handle();
		bike_online_handle();
	}
}

void bike_start(void)
{
	timer_handle_bike = xTimerCreate("bike_timer", 1000, pdTRUE, (void*)1, timer_handle_bike_cb);
	
	if(!timer_handle_bike)
	{
		Log_e("create timer failed\n");
		//防止继续执行
		while(1);
	}
	
	xTimerStart(timer_handle_bike, 0);
	
	if(xTaskCreate(bike_task, "bike_task", 256, NULL, 3, (TaskHandle_t *)&bike_task_handle) != pdPASS)
	{
		Log_e("bike_task create failed****************************************************************************\n");
		while(1);
	}
}
