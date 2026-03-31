
/*-------------------------------------------------*/
/*                                                 */
/*            实现定时器2功能的头文件              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件

#ifndef _TIMER2_H
#define _TIMER2_H

extern u8  SENG_DATA,MQ2_DATA,printf_time;
extern int times_mq;
void TIM2_ENABLE_30S(void);
extern void TIM2_Int_Init(u16 arr,u16 psc);
extern void TIM2_ENABLE_2S(void);

#endif
