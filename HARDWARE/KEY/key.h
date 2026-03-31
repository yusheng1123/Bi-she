#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"	 


#define true 1
#define false 0


typedef struct
{
	u8 k0_state;
	u8 k1_state;
	u8 k4_state;
	u8 k5_state;
	u8 k6_state;
	u8 k7_state;
	u8 k8_state;
	u8 k10_state;
	u8 k11_state;
	u8 k12_state;
	u8 k0_flag;
	u8 k1_flag;
	u8 k4_flag;
	u8 k6_flag;
	u8 k7_flag;

}KEY;

extern KEY kk;

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数				
void EXTIX_Init(void);
void key_set_data_task(void);		    
#endif


