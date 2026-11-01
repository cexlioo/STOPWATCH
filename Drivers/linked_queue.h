#ifndef __LINKED_QUEUE_H
#define __LINKED_QUEUE_H

#include "stm32f10x_conf.h"
#include "system.h"

//此队列不适合中断中调用

#define OK 0
#define ERROR -1

//定义一个通用数据结构
typedef char* data_type_t;
//链式队列的节点结构体
typedef struct queue_node {
	data_type_t data;					//数据域，使用通用数据类型
	struct queue_node *next;	//指针域，指向下一个节点
}queue_node_t;

//链式队列头节点结构体，没有数据域
typedef struct {
	queue_node_t *front;			//队头指针
	queue_node_t *rear;				//队尾指针
	SemaphoreHandle_t mutex;	//互斥锁
}queue_t;

//判空队列，空返回1，非空返回0
int is_empty(queue_t *queue);
//入队操作
int enqueue(queue_t *queue, data_type_t value);
//出队操作
int dequeue(queue_t *queue, data_type_t *value);
//查看队列元素，仅返回队头元素的值，而不改变队列的状态
int queue_head(queue_t *queue, data_type_t *value);
//初始化队列
void init_queue(queue_t *queue);

/* 测试代码 */
int queue_test_type_string(void);

#endif
