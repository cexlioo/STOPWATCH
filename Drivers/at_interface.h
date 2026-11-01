#ifndef __AT_INTERFACE_H
#define __AT_INTERFACE_H

#include "stm32f10x_conf.h"
#include "string.h"
#include "stdio.h"

typedef enum 
{
	AT_OK = 0,	//执行成功
	AT_FAILED = 1,	//执行失败
	AT_UNKNOWN_RESPONSE = 2,	//返回了未知的响应数据
	AT_TIME_OUT = 3,		//执行超时
	AT_BAD_PARAMETER = 4	//参数有误
}AT_RESULT;

AT_RESULT at_execute(char* cmd, const char* succeeded_response, const char* failed_response, 
	char* response_buffer, size_t buffer_size);

//接收字节数据
void at_receive_byte(char byte);

void at_send_cmd(char* cmd);

void at_init(void);

/**
 * 接收透传数据
 * @param buffer 缓冲区
 * @param size   缓冲区大小
 * @return 实际接收到的数据长度
 */
int at_passthrough_receive(char *buffer, int size);

/**
 * 发送透传数据
 * @param data 发送的数据
 * @param len  数据长度
 */
void at_passthrough_send(const char *data, int len);

#endif
