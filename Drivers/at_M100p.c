#include "at_M100p.h"
#include "linked_queue.h"
#include "usart.h"
#include "bike.h"

#define AT_COMMAND_MAX_LEN 128

typedef enum {
	M100P_ECHO_OFF = 0,		//对应ATE0,关闭回显
	M100P_ECHO_ON = 1			//对应ATE1,开启回显
}M100P_ECHO_MODE;

const char* RESPONSE_OK = "OK\r\n";
const char* RESPONSE_ERROR = "ERROR";
const char* RESPONSE_FAIL = "FAIL";

TaskHandle_t at_task_handle = NULL;

static char m100p_ip[16];
static uint16_t m100p_port;

static char recv_buf[512];
static int recv_data_len = 0;

recv_cb_t recv_cb; /* 函数指针 */

static queue_t cmd_queue;

#define M100P_NOT_CONNECTED_SERVER 0
#define M100P_CONNECTED_SERVER 1
static int m100p_is_connect_server = M100P_NOT_CONNECTED_SERVER;

//测试是否正常上电，准备就绪
AT_RESULT m100p_ready(void)
{
	const char* cmd = "AT\r";
	return at_execute((char*)cmd, RESPONSE_OK, RESPONSE_ERROR, NULL, 0);
}

//关闭回显
AT_RESULT m100p_set_echo(M100P_ECHO_MODE enable)
{
	const char* cmd = enable ? "ATE1\r" : "ATE0\r";
	return at_execute((char*)cmd, RESPONSE_OK, RESPONSE_ERROR, NULL, 0);
}

//测试SIM卡是否解锁
AT_RESULT m100p_sim_unlock(void)
{
	char status[10];
	char response[256];
	AT_RESULT result;
	
	result = at_execute("AT+CPIN?\r", RESPONSE_OK, RESPONSE_ERROR, response, sizeof(response));
	if(result != AT_OK)
	{
		printf("Failed to execute AT+CPIN\n");
		return result;
	}
	
	//解析完整响应
	printf("AT+CPIN Response: %s\n", response);
	char* pos = strstr(response, "+CPIN:");
	if(pos)
	{
		if(sscanf(pos, "+CPIN: %s", status) == 1)
		{
			if(strcmp(status, "READY") == 0)
			{
				return AT_OK;
			}
		}
		else
		{
			printf("AT+CPIN return wrong status: %s\n", status);
		}
	}
	
	return AT_FAILED;
}

//检测是否已注册到运营商网络
AT_RESULT m100p_register(void)
{
	char status[10];
	char response[256];
	AT_RESULT result;
	
	result = at_execute("AT+CREG?\r", RESPONSE_OK, RESPONSE_ERROR, response, sizeof(response));
	if(result != AT_OK)
	{
		printf("Failed to execute AT+CREG\n");
		return result;
	}
	
	//解析完整响应
	printf("AT+CREG Response: %s\n", response);
	char* pos = strstr(response, "+CREG:");
	if(pos)
	{
		if(sscanf(pos, "+CREG: %s", status) == 1)
		{
			if(strcmp(status, "0,1") == 0)
			{
				return AT_OK;
			}
		}
		else
		{
			printf("AT+CREG return wrong status: %s\n", status);
		}
	}
	
	return AT_FAILED;
}

//检测是否附着到GPRS网络
AT_RESULT m100p_attached(void)
{
	char status[10];
	char response[256];
	AT_RESULT result;
	
	result = at_execute("AT+CGATT?\r", RESPONSE_OK, RESPONSE_ERROR, response, sizeof(response));
	if(result != AT_OK)
	{
		printf("Failed to execute AT+CGATT\n");
		return result;
	}
	
	//解析完整响应
	printf("AT+CGATT Response: %s\n", response);
	char* pos = strstr(response, "+CGATT:");
	if(pos)
	{
		if(sscanf(pos, "+CGATT: %s", status) == 1)
		{
			if(strcmp(status, "1") == 0)
			{
				return AT_OK;
			}
		}
		else
		{
			printf("AT+CGATT return wrong status: %s\n", status);
		}
	}
	
	return AT_FAILED;
}

//设置apn
AT_RESULT m100p_set_apn(void)
{
	return at_execute("AT+CSTT\r", RESPONSE_OK, RESPONSE_ERROR, NULL, 0);
}

//激活PDP上下文
AT_RESULT m100p_activate_pdp(void)
{
	return at_execute("AT+CIICR\r", RESPONSE_OK, RESPONSE_ERROR, NULL, 0);
}

//关闭PDP上下文
AT_RESULT m100p_shut_pdp(void)
{
	return at_execute("AT+CIPSHUT\r", "SHUT OK\r\n", RESPONSE_ERROR, NULL, 0);
}

//设置透传模式
AT_RESULT m100p_set_passthrough_mode(void)
{
	AT_RESULT result;
	
	result = at_execute("AT+CIPMODE=1\r", RESPONSE_OK, RESPONSE_ERROR, NULL, 0);
	if(result != AT_OK)
	{
		printf("Failed to execute AT+CIPMODE\n");
		return result;
	}
	
	return AT_OK;
}

//退出透传模式
void m100p_exit_passthrough_mode(void)
{
	os_delay_s(1);
	at_send_cmd("+++");
	os_delay_ms(500);
}

//启动连接
AT_RESULT m100p_connect_server(const char* type, const char* ip, int port)
{
	char cmd[AT_COMMAND_MAX_LEN];
	snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"%s\",\"%s\",%d\r", type, ip, port);
	return at_execute(cmd, "CONNECT\r\n", RESPONSE_ERROR, NULL, 0);
}

//断开连接
AT_RESULT m100p_disconnect_server(void)
{
	return at_execute("AT+CIPCLOSE\r", "CLOSE OK\r\n", RESPONSE_ERROR, NULL, 0);
}

//发送数据
void m100p_send_data(const char* data, int len)
{
	if(m100p_is_connect_server == M100P_CONNECTED_SERVER)
	{
		char* str = (char*)os_malloc(len + 1);
		if(!str)
		{
			Log_e("Memory allocation failed\n");
			return;
		}
		strcpy(str, data);
		enqueue(&cmd_queue, str);
		os_mem_info();
		Log_d("enqueue address [%p]\n", str);// remember to free!
		//通知发送任务
		xTaskNotify(at_task_handle, NOTIFY_SEND_DATA, eSetBits);
	}
}

//接收数据
int m100p_recv_data(char* buffer, int size)
{
	return at_passthrough_receive(buffer, size);
}

//重启m100p
void m100p_reset(void)
{
	static int first_run = 1;
  // 第一次运行不打印，以免歧义
  first_run == 1 ? first_run = 0 : Log_e("\n***** Critical error occurred! M100P Reset *****\n");
	GPIO_SetBits(M100P_EN_PORT, M100P_EN_PIN);
	os_delay(12000);// 调度器还没启动，不能调用vTaskDelay
	GPIO_ResetBits(M100P_EN_PORT, M100P_EN_PIN);
}

//返回m100p是否连接到服务器
int m100p_is_connected_server(void)
{
	return m100p_is_connect_server;
}

//退出透传模式，断开服务器，关闭上下文
AT_RESULT m100p_disconnect_server_shut(void)
{
	AT_RESULT res;
	
	if(m100p_is_connect_server == M100P_NOT_CONNECTED_SERVER)
		return AT_OK;
	
	m100p_is_connect_server = M100P_NOT_CONNECTED_SERVER;
	
	m100p_exit_passthrough_mode();
	
	res = m100p_disconnect_server();
	if(res != AT_OK)
	{
		return res;
	}
	
	res = m100p_shut_pdp();
	if(res != AT_OK)
	{
		return res;
	}
	
	return AT_OK;
}

//配置网络并连接服务器
AT_RESULT m100p_activate_connect_server(const char* ip, int port)
{
	AT_RESULT res;
	static int n = 0;
	
	if(m100p_is_connect_server == M100P_CONNECTED_SERVER)
		return AT_OK;
	
	n++;
	
	//发送10次AT命令，测试模块是否正常启动，回应AT指令
	if(n >= 10)
	{
		n = 0;
		printf("Failed connect to server 10 times, restarting module...\n");
		m100p_reset();
		return AT_TIME_OUT;
	}
	
	res = m100p_ready();
	if(res != AT_OK)
	{
		printf("m100p_ready failed: %d\n", res);
		return res;
	}
	
	res = m100p_set_echo(M100P_ECHO_OFF);
	if(res != AT_OK)
	{
		printf("m100p_set_echo failed: %d\n", res);
		return res;
	}
	
	res = m100p_sim_unlock();
	if(res != AT_OK)
	{
		printf("m100p_sim_unlock failed: %d\n", res);
		return res;
	}
	
	res = m100p_register();
	if(res != AT_OK)
	{
		printf("m100p_register failed: %d\n", res);
		return res;
	}
	
	res = m100p_attached();
	if(res != AT_OK)
	{
		printf("m100p_attached failed: %d\n", res);
		return res;
	}
	
	res = m100p_set_passthrough_mode();
	if(res != AT_OK)
	{
		printf("m100p_set_passthrough_mode failed: %d\n", res);
		return res;
	}
	
	res = m100p_connect_server("TCP", ip, port);
	if(res != AT_OK)
	{
		printf("m100p_connect_server failed: %d\n", res);
		return res;
	}
	
	m100p_is_connect_server = M100P_CONNECTED_SERVER;
	n = 0;
	
	return AT_OK;
}

void m100p_at_task(void *pvParameters)
{
	uint32_t notify_bits = 0;
	char* data_to_send;
	
	while(1)
	{
		//透传模式时，永远阻塞在这一句
		xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t*)&notify_bits, portMAX_DELAY);
		
		if(notify_bits & NOTIFY_START_M100P)//启动模块，连接服务器
		{
			Log_d("NOTIFY_START_M100P\n");
			while(m100p_activate_connect_server(m100p_ip, m100p_port) != AT_OK)
			{
				os_delay_ms(100);
			}
			USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//使能空闲中断
			xTaskNotify(bike_task_handle, NOTIFY_CONNECTED_SERVER, eSetBits);
		}

		if(notify_bits & NOTIFY_SEND_DATA)//发送数据到服务器
		{
			Log_d("NOTIFY_SEND_DATA\n");

			while(!is_empty(&cmd_queue))
			{
				if(dequeue(&cmd_queue, &data_to_send) == OK)
				{
					//Log_d("send [%p] [%s] to server\n", data_to_send, data_to_send);
					at_passthrough_send(data_to_send, strlen(data_to_send));
					//释放字符串内存
					os_free(data_to_send);
					data_to_send = NULL;
				}
			}
		}
		
		if(notify_bits & NOTIFY_USART_RX)//从服务器收到数据
		{
			Log_d("NOTIFY_USART_RX\n");
			if(m100p_is_connect_server == M100P_CONNECTED_SERVER)
			{
				recv_data_len = at_passthrough_receive(recv_buf, sizeof(recv_buf));
				recv_buf[recv_data_len] = '\0';
				recv_cb(recv_buf, recv_data_len);
			}
		}
		
		if(notify_bits & NOTIFY_STOP_M100P)//断开连接
		{
			Log_d("NOTIFY_STOP_M100P\n");
			USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);//关闭空闲中断
			m100p_disconnect_server_shut();
		}
	}
}

//启动m100p
void m100p_start(const char* ip, int port, recv_cb_t cb)
{
	memset(m100p_ip, 0, sizeof(m100p_ip));
	strcpy(m100p_ip, ip);
	m100p_port = port;
	recv_cb = cb;
	
	xTaskNotify(at_task_handle, NOTIFY_START_M100P, eSetBits);
}

//停止m100p
void m100p_stop(void)
{
	xTaskNotify(at_task_handle, NOTIFY_STOP_M100P, eSetBits);
}

//初始化m100p的控制脚
void m100p_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(M100P_EN_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = M100P_EN_PIN;
	GPIO_Init(M100P_EN_PORT, &GPIO_InitStructure);
	
	at_init();
	m100p_reset();
	init_queue(&cmd_queue);
	
	//创建主任务
	os_mem_info();
	if(xTaskCreate(m100p_at_task, "m100p_at_task", 256, NULL, 4, (TaskHandle_t*)&at_task_handle) != pdPASS)
	{
		printf("m100p_task create failed\n");
		while(1);
	}
	os_mem_info();
	
	usart2_init();
}
