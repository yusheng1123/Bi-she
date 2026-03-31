#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "timer3.h"
#include "usart1.h"
#include "al_debug.h"
#include "beep.h"
#include "oled.h"
#include "ds1302.h"

KEY kk;
		    
//按键初始化函数
void KEY_Init(void) // IO初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); //使能PORTA,PORTE时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);		  //初始化GPIOPB5

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	  // PB4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);		  //初始化GPIOPB4
}

//==========================================================
//	函数名称：	void key_set_data_task(void)
//
//	函数功能：	按键设置
//
//	入口参数：	NO
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void key_set_data_task(void) 
{
	if(g_sysconfig.key_page == 0)
	{
		if(g_sysconfig.key_num == 1)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				g_sysconfig.mor_hour+=1;  //早上小时+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)       
			{
				Delay_Ms(20);
				g_sysconfig.mor_hour-=1;  //早上小时-
			}
		}else if(g_sysconfig.key_num == 2)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				g_sysconfig.mor_min+=1;  //早上分钟+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				g_sysconfig.mor_min-=1;  //早上分钟+
			}
		}else if(g_sysconfig.key_num == 3)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)   
			{
				Delay_Ms(20);
				g_sysconfig.nig_hour+=1;  //晚上小时+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				g_sysconfig.nig_hour-=1;  //晚上小时-
			}
		}else if(g_sysconfig.key_num == 4)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)   
			{
				Delay_Ms(20);
				g_sysconfig.nig_min+=1;  //晚上分钟+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(10);
				g_sysconfig.nig_min-=1;  //晚上分钟-
			}
		}
	}else if(g_sysconfig.key_page == 1)
	{
		if(g_sysconfig.key_num == 1)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				g_sysconfig.med_A+=1;  //药类A+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)       
			{
				Delay_Ms(20);
				g_sysconfig.med_A-=1;  //药类A-
			}
		}else if(g_sysconfig.key_num == 2)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				g_sysconfig.med_B+=1;  //药类B+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				g_sysconfig.med_B-=1;  //药类B-
			}
		}else if(g_sysconfig.key_num == 3)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				g_sysconfig.med_C+=1;  //药类C+
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				g_sysconfig.med_C-=1;  //药类C-
			}
		}
	}else if(g_sysconfig.key_page == 2)
	{
		if(g_sysconfig.key_num == 1)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.w_year+=1; 
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)       
			{
				Delay_Ms(20);
				calendar.w_year-=1; 
			}
		}else if(g_sysconfig.key_num == 2)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.w_month+=1;  
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				calendar.w_month-=1; 
			}
		}else if(g_sysconfig.key_num == 3)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.w_date+=1;  
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				calendar.w_date-=1; 
			}
		}else if(g_sysconfig.key_num == 4)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.hour+=1;  
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				calendar.hour-=1; 
			}
		}else if(g_sysconfig.key_num == 5)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.min+=1;  
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				calendar.min-=1; 
			}
		}else if(g_sysconfig.key_num == 6)
		{
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)    
			{
				Delay_Ms(20);
				calendar.sec+=1;  
			}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)      
			{
				Delay_Ms(20);
				calendar.sec-=1; 
			}
		}
	}

	//参数选择
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0)     
	{
		Delay_Ms(10);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0);     
		Delay_Ms(10);
		g_sysconfig.key_num+=1;
//		if(g_sysconfig.key_num >=4) g_sysconfig.key_num = 0;
	}

	//显示页面切换
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 0)     
	{
		Delay_Ms(10);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 0);     
		Delay_Ms(10);
		g_sysconfig.key_page+=1;
		if(g_sysconfig.key_page >=3) g_sysconfig.key_page = 0;
		OLED_Clear();
	}
	
	if(g_sysconfig.key_page == 0)
	{
		//OK确认
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0 && kk.k8_state == false)      
		{
			Delay_Ms(10);
			while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0);     
			Delay_Ms(10);
			LCD_ShowChinse_GB162(98,32,(u8 *)"OK");
			OLED_Refresh();
			bring();
			g_sysconfig.open_mor_rtc_state= 1;
			g_sysconfig.open_nig_rtc_state= 1;
			kk.k8_state = true;
		}else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0 && kk.k8_state == true)  //取消  
		{
			Delay_Ms(10);
			while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0);     
			Delay_Ms(10);
			LCD_ShowChinse_GB162(98,32,(u8 *)"  ");
			OLED_Refresh();
			g_sysconfig.open_mor_rtc_state= 0;
			g_sysconfig.open_nig_rtc_state= 0;
			kk.k8_state = false;
		}
	}else if(g_sysconfig.key_page == 2)
	{
		//OK确认
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)      
		{
			Delay_Ms(10);
			while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0);     
			Delay_Ms(10);
			
			RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec); //校准时间      
			LCD_ShowChinse_GB162(50,40,(u8 *)"OK");
			OLED_Refresh();
			Delay_Ms(1000);
			LCD_ShowChinse_GB162(50,40,(u8 *)"  ");
			OLED_Refresh();
		}
	}
}


