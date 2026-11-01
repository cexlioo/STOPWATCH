#include "flash_data.h"
#include "spi.h"
#include "w25qxx.h"
#include "string.h"
#include "stdio.h"
#include "flash_index.h"
#include "utils.h"
#include "OLED.h"

#define ITEM_CNT_IN_RECORD 13	//一条记录有几个数据项
#define RECORD_SIZE	256				//一条记录的大小

#pragma pack(push, 1)					//强制结构体的成员紧密排列
struct riding_item
{
	double lon_deg;							//实时经度原始数据，单位 度分
	double lat_deg;							//实时纬度原始数据，单位 度分
	uint16_t speed_kmh;					//实时速度
};
#pragma pack(pop)							//强制结构体的成员紧密排列

struct riding_status	//20字节
{
	uint32_t timestamp;					//UNIX时间戳
	uint16_t max_speed_kmh;			//最大时速
	uint16_t average_speed_kmh;	//平均时速
	float kcal;									//卡路里
	uint16_t average_humi;			//平均湿度
	int16_t average_temp;				//平均温度
	uint16_t total_distance_km;	//骑行距离 公里
	uint8_t reserved[1];				//保留1字节
	uint8_t write_flag;					//写入标志位 0xA5
};

//是否是写入第一条数据项，如果是则表示刚刚启动骑行
static uint8_t write_first_item = 1;
//骑行完毕应当将write_first_item 置1，为下一次骑行做准备
void set_first_write_flag(void)
{
	write_first_item = 1;
}

//获取当前骑行状态信息，并将它们转成相应的存储格式
static void get_riding_status(struct riding_status* status)
{
	status->write_flag = FLASH_WRITE_FLAG;
	status->reserved[0] = 0;
	status->total_distance_km = float_to_uint16(bike_get_distance_km());
	status->timestamp = time_str_to_timestamp(bike_get_time_str());
	status->max_speed_kmh = float_to_uint16(bike_get_max_speed_kmh());
	status->average_speed_kmh = float_to_uint16(bike_get_average_speed_kmh());
	status->kcal = bike_get_kcal();
	status->average_humi = float_to_uint16(bike_get_average_humi());
	status->average_temp = float_to_int16(bike_get_average_temp());
}

//写一条记录到flash，13个item + 1个status = 1条record
void write_record_to_flash(struct riding_item items[ITEM_CNT_IN_RECORD], struct riding_status* status)
{
	int i;
	uint8_t record_buf[RECORD_SIZE] = {0};
	uint8_t* ptr = record_buf;
	//当前写入flash的页地址
	static uint32_t current_page_addr;
	
	//将13条item写入缓冲区
	for(i = 0; i < ITEM_CNT_IN_RECORD; i++)
	{
		memcpy(ptr, &items[i], sizeof(struct riding_item));
		ptr += sizeof(struct riding_item);
	}
	
	//保留两字节
	ptr += 2;
	
	//1条status
	memcpy(ptr, status, sizeof(struct riding_status));
	//ptr += sizeof(struct riding_status);
	
	//是否是一次骑行
	if(write_first_item)
	{
		current_page_addr = get_next_table_addr();
		Log_i("item start page addr:0x%x\n", current_page_addr);
	}
	
	//判断是否需要擦除扇区
	if(current_page_addr % 4096 == 0)
	{
		Log_i("erase addr 0x%x\n", current_page_addr);
		w25qxx_sector_erase(current_page_addr);
	}
	Log_i("write item to addr 0x%x\n", current_page_addr);
	//delay(50);
	w25qxx_page_write_bytes(current_page_addr, record_buf, sizeof(record_buf));
	
	//跳到下一页
	current_page_addr += RECORD_SIZE;
	//第一次骑行
	if(write_first_item)
	{
		write_first_item = 0;
		write_index();
	}
}

void save_item(char* lat, char* lon, float speed_kmh, enum BIKE_STATE bike_state)
{
	struct riding_status status;
	struct riding_item item;
	
	static struct riding_item items[ITEM_CNT_IN_RECORD] = {0};
	static uint8_t items_idx = 0;
	
	sscanf(lat, "%lf", &item.lat_deg);
	sscanf(lon, "%lf", &item.lon_deg);
	item.speed_kmh = float_to_uint16(speed_kmh);
	
	Log_i("idx:%d lat:%lf lon:%lf speed:%u\n", 
		items_idx, item.lat_deg, item.lon_deg, item.speed_kmh);
	
	memcpy(&items[items_idx++], &item, sizeof(struct riding_item));
	
	//共写入13条，234字节
	if(items_idx == ITEM_CNT_IN_RECORD || bike_state == STOP)
	{
		get_riding_status(&status);//获取当前骑行状态数据
		write_record_to_flash(items, &status);
		//清空写入的经纬度数据
		memset(items, 0, sizeof(items));
		items_idx = 0;
	}
}

void print_status(int n, struct riding_status* status)
{
	char time_str[32] = {0};
	uint32_t start_ts = get_table_n_created_timestamp(n);
	Log_d("=========================================\n");
	Log_d("The [%dth] Riding Status Summary\n", n);
	Log_d("Start Time: %s\n", timestamp_to_time_str(start_ts, time_str));
	Log_d("End Time: %s\n", timestamp_to_time_str(status->timestamp, time_str));
	Log_d("Total Time: %s\n", format_seconds(status->timestamp - start_ts, time_str));
	Log_d("Total Distance: %.02f km\n", uint16_to_float(status->total_distance_km));
	Log_d("Max Speed: %.02f km/h\n", uint16_to_float(status->max_speed_kmh));
  Log_d("Average Speed: %.02f km/h\n", uint16_to_float(status->average_speed_kmh));
  Log_d("Calorie: %.02f KCAL\n", status->kcal);
  Log_d("Average Humidity: %.02f%%\n", uint16_to_float(status->average_humi));
  Log_d("Average Temperature: %.02f C\n", int16_to_float(status->average_temp));
  Log_d("=====================================================\n");
}

void print_item(struct riding_item* item)
{
	double lat_dec;	//十进制
	double lon_dec;	
	double gcj_lat;	//火星
	double gcj_lon;
	
	lat_dec = convert_to_decimal_degrees(item->lat_deg);
	lon_dec = convert_to_decimal_degrees(item->lon_deg);
	
	gps_to_gcj02(lat_dec, lon_dec, &gcj_lat, &gcj_lon);
	
	os_printf("{ lat: %.16lf, lng: %.16lf, speed: %.02f },\n", 
		gcj_lat, gcj_lon, uint16_to_float(item->speed_kmh));
}

//输出一次骑行数据
void print_table_n(int n)
{
	uint8_t record_buf[RECORD_SIZE] = {0};
	uint8_t zero_array[18] = {0};
	int i;
	int is_valid_data = 0;
	uint8_t npage = 0;
	uint32_t start = get_table_n_addr(n);
	uint32_t addr;
	struct riding_item* item;
	struct riding_status last_status;
	
	OLED_Clear_Part(2, 1, 16);
	OLED_ShowString(2, 1, "data exporting");
	
	while(1)
	{
		memset(record_buf, 0, sizeof(record_buf));
		
		addr = start + npage * RECORD_SIZE;
		
		w25qxx_read_bytes(addr, record_buf, RECORD_SIZE);
		
		//查看标志位
		if(record_buf[255] == FLASH_WRITE_FLAG)
		{
			if(is_valid_data == 0)
			{
				is_valid_data = 1;
				//打印开头
				os_printf("var data = [\n");
			}
			//拿出骑行状态数据
			memcpy(&last_status, &record_buf[236], sizeof(struct riding_status));
			
			item = (struct riding_item*)record_buf;
			
			OLED_Clear_Part(3, 1, 16);
			
			for(i = 0; i < ITEM_CNT_IN_RECORD; i++, item++)
			{
				//如果三个变量都是零，则是数据结尾，不再输出
				if(memcmp(item, zero_array, sizeof(struct riding_item)) == 0)
				{
					break;
				}
				print_item(item);
				OLED_ShowString(3, i + 2, ".");
			}
		}
		else
		{
			if(is_valid_data == 1)
			{
				os_printf("];\n");//打印结尾
				print_status(n, &last_status);
			}
			else
			{
        Log_w("can not find table [%d] flag, addr:0x%x\n", n, addr);
      }
			break;
		}
		
		npage++;
	}
	OLED_Clear_Part(2, 1, 16);
	OLED_Clear_Part(3, 1, 16);
}

void init_flash(void)
{
	uint8_t MID;
	uint16_t DID;
	
	spi_init();
	
	w25qxx_init();
	
	w25qxx_read_id(&MID, &DID);
	Log_d("MID:0x%X, DID:0x%X\r\n", MID, DID);
	if(MID != 0xEF)
	{
		Log_e("SPI Flash W25QXX test fail\r\n");
		while(1);//to do 未处理
	}
	
	//如果运行过上一个版本需要擦除一次flash
	//w25qxx_chip_erase();
	
	init_index();
}
