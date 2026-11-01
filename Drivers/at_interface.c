#include "at_interface.h"
#include "ringbuffer.h"
#include "usart.h"
#include "system.h"

#define AT_TIMEOUT 2000		//超时设置，2000毫秒

ringbuffer_t at_ringbuf;

/**
 * 发送透传数据
 * @param data 发送的数据
 * @param len  数据长度
 */
void at_passthrough_send(const char *data, int len)
{
  while (len--)
	{
    usart2_send_byte(*data);
    data++;
  }
}

/**
 * 接收透传数据
 * @param buffer 缓冲区
 * @param size   缓冲区大小
 * @return 实际接收到的数据长度
 */
int at_passthrough_receive(char *buffer, int size)
{
  return ringbuffer_read(&at_ringbuf, buffer, size);
}

//接收字节数据
void at_receive_byte(char byte)
{
    // 写入环形缓冲区
    ringbuffer_write(&at_ringbuf, byte);
}

void at_send_cmd(char* cmd)
{
	usart2_send_string(cmd);
	printf("at send cmd: %s\n", cmd);
}

AT_RESULT at_wait_for_response(const char* succeeded_response, const char* failed_response, int32_t timeout_ms,
	 char* response_buffer, size_t buffer_size)
{
	uint32_t start_time = os_millis();	//获取当前系统毫秒数
	char buffer[512] = {0};							//假设每次读取的数据不超过512字节
	int buffer_offset = 0;							//当前缓冲区的数据长度
	int bytes_read = 0;									//一次读取到的数据
	
	//持续读取超时
	while(1)
	{
		if(timeout_ms != -1 && (os_millis() - start_time >= timeout_ms))
		{
			//读取超时
			printf(" at_wait_for_response timeout\n");
			return AT_TIME_OUT;
		}
		
		//从缓冲区读取数据，直接写入buffer的剩余位置
		bytes_read = ringbuffer_read(&at_ringbuf, buffer + buffer_offset, sizeof(buffer) - buffer_offset - 1);
		
		if(bytes_read > 0)
		{
			//更新偏移长度
			buffer_offset += bytes_read;
			
			//检查是否包含成功响应
			if(succeeded_response && strstr(buffer, succeeded_response))
			{
				if(response_buffer)
				{
					memset(response_buffer, 0, buffer_size);
					strncpy(response_buffer, buffer, buffer_size - 1);
				}
				return AT_OK;
			}
			
			//检查是否包含失败响应
			if(failed_response && strstr(buffer, failed_response))
			{
				printf("at_wait_for_response AT_FAILED\n");
				return AT_FAILED;
			}
		}
	}
}

AT_RESULT at_execute(char* cmd, const char* succeeded_response, const char* failed_response, 
	char* response_buffer, size_t buffer_size)
{
	at_send_cmd(cmd);//发送AT指令
	return at_wait_for_response(succeeded_response, failed_response, AT_TIMEOUT, response_buffer, buffer_size);//等待并解析响应
}

void at_init(void)
{
	//初始化环形缓冲区
	ringbuffer_init(&at_ringbuf);
}
