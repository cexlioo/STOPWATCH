#ifndef __LOG_H
#define __LOG_H

#define LOG_MSG_MAX_LEN     (512)

#define Log_d(args...) os_log(__FILE__, __LINE__, LOG_DEBUG, args)
#define Log_i(args...) os_log(__FILE__, __LINE__, LOG_INFO, args)
#define Log_w(args...) os_log(__FILE__, __LINE__, LOG_WARN, args)
#define Log_e(args...) os_log(__FILE__, __LINE__, LOG_ERROR, args)

//日志输出等级
typedef enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
}LOG_LEVEL;

void log_init(LOG_LEVEL level);
//线程安全输出函数
void os_printf(const char *fmt, ...);
//线程安全输出日志
void os_log(const char *file, const int line, const int level, const char *fmt, ...);

#endif
