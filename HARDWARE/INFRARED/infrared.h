#ifndef __INFRARED_H
#define __INFRARED_H	 
#include "sys.h"


#define infrared_PORT GPIOB
#define infrared_PIN GPIO_Pin_13
#define infrared_RCC RCC_APB2Periph_GPIOB
#define INFRARED_STATE() GPIO_ReadInputDataBit(infrared_PORT,infrared_PIN)//¶ÁºìÍâµÄ×´Ì¬

//#define infrared_PIN_2 GPIO_Pin_12
//#define INFRARED_STATE_2() GPIO_ReadInputDataBit(infrared_PORT,infrared_PIN_2)//¶ÁºìÍâµÄ×´Ì¬

void infrared_INIT(void);
void infrared_get_state_task(void);

#endif 


