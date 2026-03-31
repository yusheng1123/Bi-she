#ifndef __USART3_H
#define __USART3_H	 
#include "sys.h"  
   
#define USART3_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		400					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态

//串口定义3
extern char usart3_arrary[512];
extern uint8_t usart3_index;
extern uint8_t usart3_data;

void u2_printf(char* fmt,...);

void Usart3_init(u32 bound);		//串口3初始化 
void usart2_init(u32 bound);

void RS485_Send_Data(USART_TypeDef* USARTx, char* buf, u8 len);
void Usart3_Clear(void);
void Usart3_Send_Data(char *buf, u8 len);
void read_usart3_data_show_task(void);
void USART3_SendCode(u8 *DATA,u8 len);
#endif

