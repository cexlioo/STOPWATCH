#include "usart.h"
/* 发送一个字节 */
void usart1_send_byte(uint8_t byte) 
{
    /* 发送一个字节，见库函数手册354页 */
    USART_SendData(USART1, byte);
    /* USART_GetFlagStatus函数检查指定的 USART 标志位设置与否，这里检查标志位 USART_FLAG_TXE
     * USART_FLAG_TXE是一个标志位，可以暂时这样理解它的作用：
     * 如果是0，表示USART_SendData函数发送的数据未发送完成
     * 如果是1，表示USART_SendData发送的字节byte完成了
     * 这个while的作用就是等待这个标记位变成1
     */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
/* 发送一个数组 */
void usart1_send_array(uint8_t* hex_array, uint8_t size)
{
     while (size--) {
        usart1_send_byte(*hex_array);
        hex_array++;
    }
}
/* 发送一个字符串 */
void usart1_send_string(char* str) 
{
    while(*str)
        usart1_send_byte(*str++);
}
/* 重定向fputc函数到串口，以便使用printf函数 */
int fputc(int ch, FILE *f)
{
    usart1_send_byte((uint8_t) ch);
    return ch;
}
/* 初始化外设 USART1 */
void usart1_init(void)
{      
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure; 

    RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;	         
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;	         
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;                    /* 波特率，一般使用9600或115200 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;   /* 数据位长度，一般是 8 */
    USART_InitStructure.USART_StopBits = USART_StopBits_1;        /* 停止位长度，一般是 1 */
    USART_InitStructure.USART_Parity = USART_Parity_No;           /* 校验位，一般选无 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 流控制，一般选无 */
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;    /* 发送+接收模式 */
    USART_Init(USART1, &USART_InitStructure); 

    /* 使能串口接收中断功能 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
            
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void usart2_send_byte(uint8_t byte)
{
	USART_SendData(USART2,byte);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
}

void usart2_send_array(uint8_t* hex_array,uint8_t size)
{
	while(size--)
	{
		usart2_send_byte(*hex_array);
		hex_array++;
	}
}

void usart2_send_string(char* str)
{
	while(*str)
	{
		usart2_send_byte(*str++);
	}
}
void usart2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//使能外设时钟
	RCC_APB1PeriphClockCmd(USART2_CLK,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//USART2_TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	// TX 必须是复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN;
	GPIO_Init(USART2_PORT,&GPIO_InitStructure);
	
	//USART2_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = USART2_RX_PIN;
	GPIO_Init(USART2_PORT,&GPIO_InitStructure);
	
	//配置USART
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2,&USART_InitStructure);
	
	//使能串口接收中断
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2,ENABLE);
}

//串口3
void usart3_send_byte(uint8_t byte)
{
	USART_SendData(USART3,byte);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);
}

void usart3_send_array(uint8_t* hex_array,uint8_t size)
{
	while(size--)
	{
		usart3_send_byte(*hex_array);
		hex_array++;
	}
}

void usart3_send_string(char* str)
{
	while(*str)
	{
		usart3_send_byte(*str++);
	}
}
void usart3_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	//使能外设时钟
	RCC_APB1PeriphClockCmd(USART3_CLK,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	//USART3_TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	// TX 必须是复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = USART3_TX_PIN;
	GPIO_Init(USART3_PORT,&GPIO_InitStructure);
	
	//USART3_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = USART3_RX_PIN;
	GPIO_Init(USART3_PORT,&GPIO_InitStructure);
	
	//配置USART
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART3,&USART_InitStructure);
	
	//使能串口接收中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3,ENABLE);
}
