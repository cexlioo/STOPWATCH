#ifndef __BIKE_H
#define __BIKE_H

#include "stm32f10x_conf.h "
#include "system.h"

extern TaskHandle_t bike_task_handle;

//定义通知位
#define NOTIFY_KEY1_PRESS 					(1 << 0)				//切换骑行状态
#define NOTIFY_KEY2_PRESS 					(1 << 1)				//切换界面
#define NOTIFY_GNRMC_RECV						(1 << 2)				//处理接收到的gps数据
#define NOTIFY_CONNECTED_SERVER			(1 << 3)				//连接服务器成功
#define NOTIFY_TIME_UPDATE 					(1 << 4)				//本地时间更新成功
#define NOTIFY_DATA_LIST 						(1 << 5)				//打印经纬度列表
#define NOTIFY_DATA_EXPORT 					(1 << 6)				//导出flash数据

/*
* 0:初始进入RUNNING
* 1:已连接服务器
* 2:已成功更新时间
*/
enum RUNNING_STATE {
  RUNNING_NOT_CONNECTED_SERVER,
  RUNNING_CONNECTED_SERVER,
  RUNNING_UPDATED_TIME
};
/*
* 0:空闲，上电后初始，屏幕展示：Welcome to use Guo Guo Bike
* 1:运行，骑行开始，按秒统计上述数据
* 2:停止，本次骑行结束，屏幕显示最后一次统计的数据
*/
enum BIKE_STATE {
  IDEL,
  RUNNING,
  STOP,
};

float bike_get_distance_km(void);

char* bike_get_time_str(void);

float bike_get_max_speed_kmh(void);

float bike_get_average_speed_kmh(void);

float bike_get_kcal(void);

float bike_get_average_humi(void);

float bike_get_average_temp(void);

/**
  * @brief  更改屏幕信息，按键2时调用
  * @param  无
  * @retval 无
  */
void bike_screen_change(void);
/**
  * @brief  更改运行状态，按键1时调用
  * @param  无
  * @retval 无
  */
void bike_running_state_change(void);
/**
  * @brief  初始化码表各个数据
  * @param  无
  * @retval 无
  */
void bike_init(void);

//自行车数据导出
void bike_data_export_request(char* cmd);

void bike_start(void);

#endif
