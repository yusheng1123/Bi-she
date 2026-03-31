#include "stm32f10x.h"  //包含需要的头文件
#include "usart3.h"
#include "delay.h"
#include "timer2.h"  	//包含需要的头文件
#include "timer3.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	 
#include "usart1.h"     //包含需要的头文件
#include "al_debug.h"

char Tx_command[8] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x70, 0x0D};

//串口定义3
char usart3_arrary[512];
uint8_t usart3_index;
uint8_t usart3_data;

//串口接收缓存区 	
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //发送缓冲,最大USART3_MAX_SEND_LEN字节

//通过判断接收连续2个字符之间的时间差不大于10ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过10ms,则认为不是1次连续数据.也就是超过10ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
vu16 USART3_RX_STA=0; 


void Usart3_Clear(void)
{
	memset(s.usart3_buff,0,sizeof(s.usart3_buff));
	s.usart3_index = 0;
	s.usart3_recv = 0;
}

void USART3_IRQHandler(void)
{
	u8 Res;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART3);
//		USART_SendData(USART1,Res);
		if (usart3_index == 0)
		{
			USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
		}
		usart3_arrary[usart3_index] = Res;
		usart3_index++;
		usart3_index %= sizeof(usart3_arrary);
	}
	else if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		usart3_data = 1;
		USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);
	}	 											 
}

//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void Usart3_init(u32 bound)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//PB10	TXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PB11	RXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
	USART_InitStructure.USART_Parity = USART_Parity_No;									//无校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;								//1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;							//8位数据位
	USART_Init(USART3, &USART_InitStructure);
	
	USART_Cmd(USART3, ENABLE);														//使能串口
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									//使能接收中断
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
}

void Usart3_Send_Data(char *buf, u8 len)
{
	u8 t;
	for (t = 0; t < len; t++) //循环发送数据
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
			;
		USART_SendData(USART3, buf[t]);
	}
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
		;
}

void USART3_SendCode(u8 *DATA,u8 len)
{
	USART_ClearFlag(USART3,USART_FLAG_TC);    //发送之前清空发送标志  没有这一句 很容易丢包 第一个数据容易丢失
	while(len--)
	{
		USART_SendData(USART3, *DATA++);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);//等待发送结束
	}
}



