#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include "stm32f10x_conf.h"
#include <stdio.h>

#define OK 0          /* 成功 */
#define ERROR -1      /* 失败 */

#define MAX_SIZE 2048  /* 定义环形缓冲区的最大容量 */

/* 定义环形缓冲区结构体 */
typedef struct {
    uint8_t buffer[MAX_SIZE];   /* 环形缓冲区 */
    int read_index;             /* 读指针 */
    int write_index;            /* 写指针 */
} ringbuffer_t;

/* 初始化环形缓冲区 */
void ringbuffer_init(ringbuffer_t* rbuf);
/* 判空操作，如果是空返回1，非空返回0 */
int ringbuffer_is_empty(ringbuffer_t* rbuf);
/* 判满操作，如果是满返回1，不满返回0 */
int ringbuffer_is_full(ringbuffer_t* rbuf);
/* 写入数据 */
int ringbuffer_write(ringbuffer_t* rbuf, char value);
/* 从环形缓冲区读取多个字节，返回读到的长度 */
int ringbuffer_read(ringbuffer_t* rbuf, char* dest, int dest_sz);
/* 查看当前缓冲区头部数据（不移除） */
int ringbuffer_peek(ringbuffer_t* rbuf, char* value);
/* 用于测试 */
int ringbuffer_test(void);

#endif
