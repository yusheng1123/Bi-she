#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


//#define WHITE_LED PAout(11)	
#define RED_LED PBout(8)	

#define LED0 PCout(13)	

void LED_Init(void);		//≥ı ºªØ

void LED_White_ON(void);
void LED_White_OFF(void);

void LED_Violet_ON(void);
void LED_Violet_OFF(void);

	 				    
#endif


