#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t log_mutex;

static char *level_str[] = {
	"DBG", "INF", "WAR", "ERR",
};

//__FILE__中包含了文件夹的名称，需要去掉文件夹
static const char *_get_filename(const char *p)
{
	char ch = '\\';
	const char *q = strrchr(p, ch);
	if(q == NULL)
	{
		q = p;
	}
	else
	{
		q++;
	}
	return q;
}

//全局日志输出标志，只能输出小于或等于该等级的日志信息
static LOG_LEVEL log_level = LOG_DEBUG;
static char log_buf[LOG_MSG_MAX_LEN];
//线程安全输出日志
void os_log(const char *file, const int line, const int level, const char *fmt, ...)
{
	int offset = 0;
	
	if(level < log_level)
	{
		return;
	}
	
	//获取源文件名
	const char *file_name = _get_filename(file);
	
	//保护串口输出，防止任务切换导致数据错乱
	xSemaphoreTake(log_mutex, portMAX_DELAY);
	
	//先把日志前缀写入buf
	offset = sprintf(log_buf, "%s|%s(%d):", level_str[level], file_name, line);
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(log_buf + offset, sizeof(log_buf) - offset, fmt, args);//从offset开始继续写
	va_end(args);
	
	usart1_send_string(log_buf);
	
	xSemaphoreGive(log_mutex);
}

//线程安全输出函数
void os_printf(const char *fmt, ...)
{
	//保护串口输出
	xSemaphoreTake(log_mutex, portMAX_DELAY);
	
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);//  vprintf 处理 va_list，输出到串口
	va_end(args);
	
	xSemaphoreGive(log_mutex);
}

void log_init(LOG_LEVEL level)
{
	usart1_init();
	
	log_level = level;
	
	log_mutex = xSemaphoreCreateMutex();
	if(!log_mutex)
	{
		printf("log init  wrong!\n");
	}
}
