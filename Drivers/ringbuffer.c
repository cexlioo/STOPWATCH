#include "ringbuffer.h"

/* 初始化环形缓冲区 */
void ringbuffer_init(ringbuffer_t* rbuf)
{
    rbuf->read_index = 0;
    rbuf->write_index = 0;
}

/* 判空操作，如果是空返回1，非空返回0 */
int ringbuffer_is_empty(ringbuffer_t* rbuf)
{
    return rbuf->read_index == rbuf->write_index;
}

/* 判满操作，如果是满返回1，不满返回0 */
int ringbuffer_is_full(ringbuffer_t* rbuf)
{
    return (rbuf->write_index + 1) % MAX_SIZE == rbuf->read_index;
}

/* 写入数据 */
int ringbuffer_write(ringbuffer_t* rbuf, char value)
{
    if (ringbuffer_is_full(rbuf)) {
        printf("RingBuffer is full!\n");
        return ERROR;
    }
    rbuf->buffer[rbuf->write_index] = value;
    /* 循环缓冲区的写指针下标 */
    rbuf->write_index = (rbuf->write_index + 1) % MAX_SIZE;
    return OK;
}

/* 从环形缓冲区读取多个字节，返回读到的长度 */
int ringbuffer_read(ringbuffer_t* rbuf, char* dest, int dest_sz)
{
    if (ringbuffer_is_empty(rbuf)) {
        return 0;  // 返回 0 表示未读取任何数据
    }

    int bytes_read = 0;

    // 持续读取，直到读到指定的大小或缓冲区为空
    while (bytes_read < dest_sz && !ringbuffer_is_empty(rbuf)) {
        dest[bytes_read] = rbuf->buffer[rbuf->read_index];
        rbuf->read_index = (rbuf->read_index + 1) % MAX_SIZE; // 更新读指针
        bytes_read++;
    }

    return bytes_read;  // 返回实际读取的字节数
}

/* 查看当前缓冲区头部数据（不移除） */
int ringbuffer_peek(ringbuffer_t* rbuf, char* value)
{
    if (ringbuffer_is_empty(rbuf)) {
        printf("RingBuffer is empty!\n");
        return ERROR;
    }
    *value = rbuf->buffer[rbuf->read_index];
    return OK;
}

int ringbuffer_test(void)
{
    ringbuffer_t rbuf;  // 定义一个环形缓冲区

    /* 初始化环形缓冲区 */
    ringbuffer_init(&rbuf);

    /* 判空测试 */
    if (ringbuffer_is_empty(&rbuf)) {
        printf("RingBuffer is empty\n\n");
    }
    else {
        printf("RingBuffer not empty\n\n");
    }

    /* 写入一些数据 */
    printf("add [%c] into ringbuffer\n", 'A');
    ringbuffer_write(&rbuf, 'A');
    printf("add [%c] into ringbuffer\n", 'B');
    ringbuffer_write(&rbuf, 'B');
    printf("add [%c] into ringbuffer\n", 'C');
    ringbuffer_write(&rbuf, 'C');
    printf("add [%c] into ringbuffer\n", 'D');
    ringbuffer_write(&rbuf, 'D');

    /* 查看头部数据 */
    char value;
    if (ringbuffer_peek(&rbuf, &value) == OK) {
        printf("front value of ringbuffer is [%c]\n", value);
    }

    /* 读取所有数据 */
    while (!ringbuffer_is_empty(&rbuf)) {
        ringbuffer_read(&rbuf, &value, 1);
        printf("read value [%c] from ringbuffer\n", value);
    }

    /* 再次判空 */
    if (ringbuffer_is_empty(&rbuf)) {
        printf("RingBuffer is empty\n");
    }

    return 0;
}
