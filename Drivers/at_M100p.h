#ifndef __AT_M100P_H
#define __AT_M100P_H

#include "at_interface.h"

#define M100P_EN_PORT 	GPIOA
#define M100P_EN_CLK 		RCC_APB2Periph_GPIOA
#define M100P_EN_PIN 		GPIO_Pin_1

//定义通知位
#define NOTIFY_SEND_DATA (1 << 2)		//发送数据到服务端通知
#define NOTIFY_USART_RX (1 << 3)		//从服务端收到数据通知
#define NOTIFY_START_M100P (1 << 4)	//连接服务端通知
#define NOTIFY_STOP_M100P (1 << 5)	//断开服务端通知

/* 定义一个函数指针类型，用于接收串口透传数据时进行回调 */
typedef void(*recv_cb_t)(const char *, int);

//返回m100p是否连接到服务器
int m100p_is_connected_server(void);

//初始化m100p的控制脚
void m100p_init(void);

//发送数据
void m100p_send_data(const char* data, int len);

//启动m100p
void m100p_start(const char* ip, int port, recv_cb_t cb);

//停止m100p
void m100p_stop(void);



#endif
