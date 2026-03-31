

#include "stm32f10x.h"
#include "sys.h"

#ifndef _TIMER1_H
#define _TIMER1_H

void TIM1_ENABLE_60S(void);

#define  MAX_TIMER1            4           				// 最大定时器个数
extern volatile unsigned long    g_Timer1[MAX_TIMER1];
#define  SendTimer1        g_Timer1[0]  							// LED翻转定时器
#define  DebugTimer1  		g_Timer1[1]  							// 温度采集定时器
#define  CheckTimer1  			g_Timer1[2]  							// 门采集定时器


#define  TIMER1_SEC				(1)              // 秒
#define  TIMER1_MIN				(TIMER1_SEC*60)  // 分
#define	 TIMER1_HUOR			(TIMER1_MIN*60)	 // 时
#define	 TIMER1_DAY				(TIMER1_HUOR*24)	 // 时

extern u8 tft_reflash_falg,tft_reflash_time;

extern void TIM1_Init(uint16_t arr, uint16_t psc);
#endif
