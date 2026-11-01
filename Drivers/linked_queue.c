#include "linked_queue.h"
#include <stdio.h>
#include <stdlib.h>

//判空队列，空返回1，非空返回0
int is_empty(queue_t *queue)
{
	return queue->front == NULL;
}

//入队操作
int enqueue(queue_t *queue, data_type_t value)
{
	queue_node_t *new_node = (queue_node_t*)os_malloc(sizeof(queue_node_t));
	if(!new_node)
	{
		return ERROR;//内存分配失败
	}
	new_node->data = value;
	new_node->next = NULL;
	
	//加锁
	xSemaphoreTake(queue->mutex, portMAX_DELAY);
	
	if(is_empty(queue))
	{
		queue->front = queue->rear = new_node;
	}
	else
	{
		queue->rear->next = new_node;
		queue->rear = new_node;
	}
	
	//解锁
	xSemaphoreGive(queue->mutex);
	
	return OK;
}	

//出队操作
int dequeue(queue_t *queue, data_type_t *value)
{
	queue_node_t *del_node;
	
	//加锁
	xSemaphoreTake(queue->mutex, portMAX_DELAY);
	
	if(is_empty(queue))
	{
		xSemaphoreGive(queue->mutex);
		return ERROR;
	}
	
	del_node = queue->front;
	*value = del_node->data;
	queue->front = queue->front->next;
	if(queue->front == NULL)//没有元素了
	{
		queue->rear = NULL;
	}
	
	xSemaphoreGive(queue->mutex);
	//释放节点
	os_free(del_node);
	
	return OK;
}	

//查看队列元素，仅返回队头元素的值，而不改变队列的状态
int queue_head(queue_t *queue, data_type_t *value)
{
	xSemaphoreTake(queue->mutex, portMAX_DELAY);
	
	if(is_empty(queue))
	{
		xSemaphoreGive(queue->mutex);
		return ERROR;
	}
	*value = queue->front->data;
	
	xSemaphoreGive(queue->mutex);
	
	return OK;
}
//初始化队列
void init_queue(queue_t *queue)
{
	queue->front = queue->rear = NULL;
	queue->mutex = xSemaphoreCreateMutex();//创建互斥锁
	if(queue->mutex == NULL)
	{
		Log_e("init queue failed!\n");
		while(1);
	}
}

/* 假设每个字符串最大长度为 32 */
#define STR_BUF_LEN 32

/* 测试代码 */
int queue_test_type_string(void)
{
    queue_t queue;
    char* value;
    int i;

    /* 初始化队列 */
    init_queue(&queue);

    if (is_empty(&queue)) {
        printf("Queue is empty\n\n");
    } else {
        printf("Queue not empty\n\n");
    }

    /* 增加一些字符串元素 */
    const char *test_data[] = {"apple", "banana", "cherry", "date"};
    for (i = 0; i < sizeof(test_data)/sizeof(test_data[0]); i++) {
        char *str = (char *)os_malloc(STR_BUF_LEN);
        if (!str) {
            printf("Memory allocation failed\n");
            return -1;
        }
        snprintf(str, STR_BUF_LEN, "%s", test_data[i]);
        printf("add [%s] into queue\n", str);
        enqueue(&queue, str);
    }

    if (queue_head(&queue, &value) == OK) {
        printf("front value of queue is [%s]\n", value);
    }

    while (!is_empty(&queue)) {
        if (dequeue(&queue, &value) == OK) {
            printf("dequeue value [%s]\n", value);
            os_free(value);  // 释放字符串内存
        }
    }

    return 0;
}

