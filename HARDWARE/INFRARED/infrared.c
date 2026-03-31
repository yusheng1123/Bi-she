#include "infrared.h"
#include "al_debug.h"
#include "beep.h"
#include "led.h"
#include "usart1.h"

//红外初始化函数
void infrared_INIT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义结构体变量
	
	RCC_APB2PeriphClockCmd(infrared_RCC,ENABLE);	
	GPIO_InitStructure.GPIO_Pin=infrared_PIN;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;	 //设置浮空输入
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(infrared_PORT,&GPIO_InitStructure); 	   /* 初始化GPIO */
}

/*
************************************************************
*	函数名称：	infrared_get_state_task
*
*	函数功能：	红外状态数据获判断   检测是否有人
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：
************************************************************
*/
void infrared_get_state_task(void)
{
	if(INFRARED_STATE() == 0 && g_sysconfig.infrared_flag == 0)
	{
		g_sysconfig.infrared_flag = 1;
	}
	else if(INFRARED_STATE() == 1 && g_sysconfig.infrared_flag == 1) 
	{
		g_sysconfig.infrared_flag = 0;
	}
}
