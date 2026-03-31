//硬件驱动
#include "stm32f10x.h"
#include "al_debug.h"
#include "delay.h"

//C库
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart1.h"     //包含需要的头文件

CCLK cclk;
al_sysconfig_t g_sysconfig;

/*
************************************************************
*	函数名称：	UsartPrintf
*
*	函数功能：	格式化打印
*
*	入口参数：	USARTx：串口组
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void AL_DEBUG_LOG(USART_TypeDef* USARTx, char* fmt, ...)
{
    if(AL_DEBUG_FILE_DEBUG)
    {
        unsigned char UsartPrintfBuf[1024];
        va_list ap;
        unsigned char* pStr = UsartPrintfBuf;

        va_start(ap, fmt);
        vsnprintf((char*)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
        va_end(ap);

        while(*pStr != 0)
        {
					 USART_ClearFlag(USARTx,USART_FLAG_TC);
           USART_SendData(USARTx, *pStr++);
           while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
        }
    }
}

void al_main_task_log_start(void)
{
	u1_printf(" Application version: %s\r\n",g_version);
	//delay_ms(1000);
}


void split_data(char *src,const char *separator,char **dest,int *num) 
{
	/*
		src 源字符串的首地址(buf的地址) 
		separator 指定的分割字符
		dest 接收子字符串的数组
		num 分割后子字符串的个数
	*/
     char *pNext;
     int count = 0;
     if (src == NULL || strlen(src) == 0) //如果传入的地址为空或长度为0，直接终止 
        return;
     if (separator == NULL || strlen(separator) == 0) //如未指定分割的字符串，直接终止 
        return;
     pNext = (char *)strtok(src,separator); //必须使用(char *)进行强制类型转换(虽然不写有的编译器中不会出现指针错误)
     while(pNext != NULL) {
          *dest++ = pNext;
          ++count;
         pNext = (char *)strtok(NULL,separator);  //必须使用(char *)进行强制类型转换
    }  
    *num = count;
}

/*
* 取两个分隔符之间的子串
* str  原始字符串
* separator1  第一个分隔符
* separator2  第二个分隔符
* num  第一个分隔符出现num次时，作为第一个分隔符的位置。
* substr  取出的两个分隔符间的子串，从0起数。"+QMTRECV: 0,0,"Hello",hello world"
*/
uint8_t get_sub_str(char *str, char *separator1, char *separator2, int8_t num, char *substr)
{
	int8_t i;
	char *p1,*p2;

	p1 = str;
	for(i = 0; i<=num; i++) {
			p1 = strstr(p1,separator1);
			if(p1 == NULL) return 1;
			p1++;
	}

	p2 = strstr(p1,separator2);
	if(p2 == NULL) return 2;
	memcpy(substr,p1,p2-p1);

	return 0;
}

/******************解析 数据****************
形参：u8* buff原始数据
			u8 *Weather_stat数据标志
			u8 *data解析获取到的数据
返回值：0---成功，其他值---失败
************************************************/
u8 Weather_analysis(u8* buff,u8 *Weather_stat,u8 *data)
{
	char *p=NULL;
	u16 i=0;
	p=strstr((char *)buff,(char *)Weather_stat);
	if(p)
	{
		p+=strlen((char *)Weather_stat)+2;
		i=0;
		while(*p!='}' && *p!='\0')
		{
			data[i++]=*p++;
		}
		data[i]='\0';
		return 0;
	}
	else return 1;
}

u8 Weather_analysis_timer(u8* buff, u8 *Time_stat, u8 *hour_min)
{
    char *p = NULL;
    // 1. 精准查找"timer":"（避免匹配到其他含timer的字符）
    char find_str[16] = {0};
    sprintf(find_str, "%s\":\"", Time_stat); // 拼接成"timer":"
    p = strstr((char *)buff, find_str);
    
    if (p == NULL)
    {
        return 1;  // 未找到timer，返回1
    }

    // 2. 跳过"timer":"，精准指向时间起始位（如09:28:00的0）
    p += strlen(find_str);

    // 3. 读取完整小时字符串（直到冒号，适配09/9等格式）
    char h[3] = {0};
    u8 i = 0;
    while (*p != ':' && *p != '\0' && i < 2) // 遇到冒号才停止读取小时
    {
        if (*p >= '0' && *p <= '9') // 只读取数字字符
        {
            h[i++] = *p;
        }
        p++;
    }

    // 4. 跳过冒号，读取完整分钟字符串
    p++; // 跳过小时后的冒号
    char m[3] = {0};
    i = 0;
    while (*p != ':' && *p != '"' && *p != '\0' && i < 2) // 遇到秒数的冒号/引号停止
    {
        if (*p >= '0' && *p <= '9') // 只读取数字字符
        {
            m[i++] = *p;
        }
        p++;
    }

    // 5. 转换为数值（自动处理09→9）
    hour_min[0] = atoi(h); // 09→9，9→9
    hour_min[1] = atoi(m); // 28→28，8→8
    return 0;
}

