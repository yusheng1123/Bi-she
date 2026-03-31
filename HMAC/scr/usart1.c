/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*             实现串口1功能的源文件               */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "usart1.h"     //包含需要的头文件

extern u8 sys_connet_aliyun_flag;

Serial s;

#if  USART1_RX_ENABLE                   //如果使能接收功能
char Usart1_RxCompleted = 0;            //定义一个变量 0：表示接收未完成 1：表示接收完成
unsigned int Usart1_RxCounter = 0;      //定义一个变量，记录串口1总共接收了多少字节的数据
char Usart1_RxBuff[USART1_RXBUFF_SIZE]; //定义一个数组，用于保存串口1接收到的数据
#endif

/*-------------------------------------------------*/
/*函数名：初始化串口1发送功能                      */
/*参  数：bound：波特率                            */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Usart1_Init(unsigned int bound)
{
	GPIO_InitTypeDef gpio_initstruct;
	USART_InitTypeDef usart_initstruct;
	NVIC_InitTypeDef nvic_initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//PA9	TXD
	gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_9;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);
	
	//PA10	RXD
	gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_10;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);
	
	usart_initstruct.USART_BaudRate = bound;
	usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
	usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
	usart_initstruct.USART_Parity = USART_Parity_No;									//无校验
	usart_initstruct.USART_StopBits = USART_StopBits_1;								//1位停止位
	usart_initstruct.USART_WordLength = USART_WordLength_8b;							//8位数据位
	USART_Init(USART1, &usart_initstruct);
	
	USART_Cmd(USART1, ENABLE);														//使能串口
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);									//使能接收中断
	
	nvic_initstruct.NVIC_IRQChannel = USART1_IRQn;
	nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_initstruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&nvic_initstruct);
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断
	{		
		if(s.usart_recv == false)
		{
			USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
		}				
		
		if(s.usart_index >= sizeof(s.usart_buff))	s.usart_index = 0; //防止串口被刷爆
		
		s.usart_buff[s.usart_index++] = USART1->DR;
		s.usart_index %= sizeof(s.usart_buff);
		USART_ClearFlag(USART1, USART_FLAG_RXNE);
	}
	else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)	
	{
		s.usart_recv = true;
		USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);//关闭
	}
}


void Usart_Clear(void)
{
    memset(s.usart_buff, 0, sizeof(s.usart_buff));
    s.usart_index = 0;
    s.usart_recv = 0;
}

/*-------------------------------------------------*/
/*函数名：串口1 printf函数                         */
/*参  数：char* fmt,...  格式化输出字符串和参数    */
/*返回值：无                                       */
/*-------------------------------------------------*/

__align(8) char Usart1_TxBuff[USART1_TXBUFF_SIZE];

void u1_printf(char* fmt, ...)
{
    unsigned int i, length;

    va_list ap;
    va_start(ap, fmt);
    vsprintf(Usart1_TxBuff, fmt, ap);
    va_end(ap);

    length = strlen((const char*)Usart1_TxBuff);
    while((USART1->SR & 0X40) == 0);
    for(i = 0; i < length; i ++)
    {
        USART1->DR = Usart1_TxBuff[i];
        while((USART1->SR & 0X40) == 0);
    }
}



