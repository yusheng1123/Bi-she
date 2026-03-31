#include "led.h"
#include "al_debug.h"
     
//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);	 																										//使能PB端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //BEEP--> 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 //根据参数初始化
 GPIO_SetBits(GPIOB, GPIO_Pin_8); 
	
// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 
// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 																											//推挽输出
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 																											//IO口速度为50MHz
// GPIO_Init(GPIOC, &GPIO_InitStructure);	
// GPIO_SetBits(GPIOC, GPIO_Pin_13); 
}




