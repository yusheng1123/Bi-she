#ifndef _TIMER3_H
#define _TIMER3_H
#include "sys.h"

extern u8  cj_flag,open_relay_flag,TIMER_ENABLE_COUNT;
extern int TIMER3_DATA,open_wind_count;

void TIM3_ENABLE_30S(void);
void TIM3_ENABLE_2S(void);
void TIM3_Init(unsigned short int, unsigned short int);
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM5_Init(u16 arr,u16 psc);
void open_sg(void);
void close_sg(void);
#endif
