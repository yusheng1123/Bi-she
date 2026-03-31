
/*-------------------------------------------------*/
/*                                                 */
/*             实现串口2功能的头文件               */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __USART2_H
#define __USART2_H

#include "stdio.h"      //包含需要的头文件
#include "stdarg.h"		//包含需要的头文件 
#include "string.h"     //包含需要的头文件
#include "stm32f10x.h"

#define USART2_RX_ENABLE     1      //是否开启接收功能  1：开启  0：关闭
#define USART2_TXBUFF_SIZE   512   //定义串口2 发送缓冲区大小 1024字节

#if  USART2_RX_ENABLE                          //如果使能接收功能
#define USART2_RXBUFF_SIZE   512              //定义串口2 接收缓冲区大小 1024字节
extern char Usart2_RxCompleted ;               //外部声明，其他文件可以调用该变量
extern unsigned int Usart2_RxCounter;          //外部声明，其他文件可以调用该变量
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //外部声明，其他文件可以调用该变量
#endif

//串口定义2
extern char usart2_arrary[512];
extern uint8_t usart2_index;
extern uint8_t usart2_data;

void Usart2_Init(unsigned int);       
void u2_printf(char*,...) ;          
void u2_TxData(unsigned char *data);
void Uart2_SendStr(char*SendBuf);
void Usartx_SendData(USART_TypeDef *USARTx,u8 *str,u32 len);
void Usartx_SendString(USART_TypeDef *USARTx,u8 *str);
#endif


