//包含需要的头文件
#include "main.h"

int main(void)
{
	g_sysconfig.mor_hour = 8;
	g_sysconfig.mor_min = 30;
	g_sysconfig.nig_hour = 18;
	g_sysconfig.nig_min = 30;
	g_sysconfig.temp_t = 35; 
	g_sysconfig.humi_t = 90;
	g_sysconfig.med_A = 2;
	g_sysconfig.med_B = 3;
	g_sysconfig.med_C = 2;
	g_sysconfig.weight_t = 10;
	g_sysconfig.open_mor_rtc_state = g_sysconfig.open_nig_rtc_state = 0;
	
	Delay_Init();                   			//延时功能初始化
	TIM1_Init(2000-1, 36000-1);						//定时器1初始化
	TIM2_Int_Init(9999,7199);             //定时器2初始化
	TIM3_Int_Init(9999, 7199);            //TIM3初始化
	Usart1_Init(115200);            			//串口1功能初始化，波特率115200,PA9 PA10,ch340,向外传输内容
	Usart2_Init(115200);            			//串口2功能初始化，波特率115200	，PA2 PA3 esp8266模块与单片机连接
	T_Adc_Init();													//ADC数模转换器初始化
	KEY_Init();														//按键初始化
	LED_Init();														//灯初始化
	BEEP_Init();                           //蜂鸣器模块
	DHT11_Init();                          //温湿度模块
	infrared_INIT();                       //初始化红外模块
	Ds1302_Init();//时钟模块初始化
  Init_HX711pin();											//重量模块初始化
	OLED_Init();										 			//OLED初始化
	OLED_Refresh();  
	al_debug_log("系统初始化成功");
	WiFi_Connect_IoTServer();            			//WiFi连接云平台
	Get_Maopi();				//称毛皮重量
	Delay_Ms(2000);
	Get_Maopi();				//重新获取毛皮重量
	OLED_Init();										 			//OLED初始化
  OLED_Refresh(); 
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx	
	
	while(1)                        		//主循环
	{		
		system_timer_data_task();
		usart1_receive_data_task();
		DHT11_Read_Data(&g_sysconfig.temperature,&g_sysconfig.humidity);	    //读取温湿度值		
		infrared_get_state_task();		    //获取红外状态
		Get_Weight();                     //重量数据获取
		key_set_data_task();             //按键设置
		oled_show_data_task();           //显示屏显示
		OLED_Refresh(); 
		sys_polcie_task();
		sys_debug_task();                //本地调试函数
		sys_publish_data();					//往云平台发送数据
		sys_reve_thingscloud_data();  //接收平台下发的指令
	}
}

void sys_publish_data(void)		//往云平台发送数据
{
	char temp[512];
	if(SendTimer1) return;
	SendTimer1 = 5 * TIMER1_SEC;
	
	sprintf(temp,"{\"temp\":%d,\"humi\":%d,\"weight\":%d,\"nor1\":%d,\"nor2\":%d,\"nor3\":%d,\"nor4\":%d,\"temp_t\":%d,\"humi_t\":%d,\"weight_t\":%d,\"med_A\":%d,\"med_B\":%d,\"med_C\":%d,\"open_mor_rtc_state\":%d,\"open_nig_rtc_state\":%d}",
	g_sysconfig.temperature,g_sysconfig.humidity,g_sysconfig.weight,g_sysconfig.nor1,g_sysconfig.nor2,g_sysconfig.nor3,g_sysconfig.nor4,g_sysconfig.temp_t,g_sysconfig.humi_t,g_sysconfig.weight_t,
	g_sysconfig.med_A,g_sysconfig.med_B,g_sysconfig.med_C,g_sysconfig.open_mor_rtc_state,g_sysconfig.open_nig_rtc_state);//发布数据给服务器 
	MQTT_PublishData(POST_TOPIC,temp,0);
	
	al_debug_log("发送数据成功");	
}

void sys_debug_task(void)				//调试任务
{
	if(DebugTimer1)	return;
	DebugTimer1 = 1 * TIMER1_SEC;
	
	al_debug_log("温度：%d,湿度：%d,药品重量：%d,红外：%d",g_sysconfig.temperature,g_sysconfig.humidity,g_sysconfig.weight,g_sysconfig.infrared_flag);
}

//参数阈值判断
void sys_polcie_task(void)
{
	//药物不足提示
	if(g_sysconfig.weight !=0 && g_sysconfig.weight < (g_sysconfig.weight_t - HYSTERESIS))
	{
		g_sysconfig.nor2 = 1;
	}else if(g_sysconfig.weight > (g_sysconfig.weight_t + HYSTERESIS))
	{
		g_sysconfig.nor2 = 0;
	}
	
	//温度报警提示
	if(g_sysconfig.temperature > (g_sysconfig.temp_t + HYSTERESIS))
	{
		g_sysconfig.nor3 = 1;
	}else if(g_sysconfig.temperature < (g_sysconfig.temp_t - HYSTERESIS))
	{
		g_sysconfig.nor3 = 0;
	}
	
	//湿度报警提示
	if(g_sysconfig.humidity > (g_sysconfig.humi_t + HYSTERESIS))
	{
		g_sysconfig.nor4 = 1;
	}else if(g_sysconfig.humidity < (g_sysconfig.humi_t - HYSTERESIS))
	{
		g_sysconfig.nor4 = 0;
	}
	
	if((g_sysconfig.nor2 | g_sysconfig.nor3 | g_sysconfig.nor4) && g_sysconfig.fan_state == false)
	{
		RED_LED = 0;
		g_sysconfig.fan_state = true;
	}else if(g_sysconfig.nor2 == 0 && g_sysconfig.nor3 == 0 && g_sysconfig.nor4 == 0 && g_sysconfig.fan_state == true)
	{
		RED_LED = 1;
		g_sysconfig.fan_state = false;
	}
}

/*
************************************************************
*	函数名称：	system_timer_data_task
*
*	函数功能：	判断是否到达吃药时间
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
void system_timer_data_task(void) 
{
	if(g_sysconfig.t != calendar.sec)
	{
		 g_sysconfig.t = calendar.sec;
		 sprintf(g_sysconfig.timer_rh,"%02d:%02d",calendar.hour, calendar.min);	
		 if(g_sysconfig.open_mor_rtc_state)
		 {
			 if((strncmp(g_sysconfig.timer_rh,morning_timer,strlen(g_sysconfig.timer_rh)) == 0 && strlen(morning_timer) >0) && g_sysconfig.open_eat_state == 0) 
			 {
				 g_sysconfig.open_eat_state = 1;
				 g_sysconfig.eat_init_weight = g_sysconfig.weight;
				 g_sysconfig.nor1 = 1;  //吃药提醒 APP标记
				 BEEP = 0;
				 RED_LED = 0;
				 g_sysconfig.open_mor_rtc_state = 0;
			 }
		 }
	 }
	if(g_sysconfig.open_nig_rtc_state)
	{
		 if((strncmp(g_sysconfig.timer_rh,night_timer,strlen(g_sysconfig.timer_rh)) == 0 && strlen(night_timer) >0) && g_sysconfig.open_eat_state == 0) 
		 {
			 g_sysconfig.open_eat_state = 1;
			 g_sysconfig.eat_init_weight = g_sysconfig.weight;
			 g_sysconfig.nor1 = 1;  //吃药提醒 APP标记
			 BEEP = 0;
			 RED_LED = 0;
			 g_sysconfig.open_nig_rtc_state = 0;
		 }
	}
	
	if(g_sysconfig.infrared_flag == 0 && g_sysconfig.open_flag == 0) //判断开药盒后，重量是否发生变化，没变化接着提示
	{
		if(g_sysconfig.weight < g_sysconfig.eat_init_weight)
		{
			BEEP = 1;
			RED_LED = 1;
			g_sysconfig.open_flag = 1;//标记
			TIMER_ENABLE_COUNT = 1;
			g_sysconfig.nor1 = 0;
		}
	}
	
	if(g_sysconfig.open_flag && cj_flag)
	{
		if(g_sysconfig.open_eat_state == 1 && g_sysconfig.infrared_flag == 0) //提示药箱没有关
		{
			BEEP = 0;
			RED_LED = !RED_LED;
		}
	}
	
	if(g_sysconfig.key_page == 0)
	{
		if(g_sysconfig.open_mor_rtc_state == 0 && g_sysconfig.open_nig_rtc_state == 0)
		{
			LCD_ShowChinse_GB162(98,32,(u8 *)"  ");
			OLED_Refresh();
			kk.k8_state = false;
		}
	}
	
	if(g_sysconfig.infrared_flag && g_sysconfig.open_flag) //吃完药且关闭药盒
	{
		BEEP = 1;
	  RED_LED = 1;
		g_sysconfig.open_eat_state = 0;
		g_sysconfig.open_flag = 0;
		TIMER_ENABLE_COUNT = 0;
		open_wind_count = 0;
		cj_flag = 0;
	}
}

//==========================================================
//	函数名称：	oled_show_data_task
//
//	函数功能：	OLED显示屏显示数据
//
//	入口参数：	NO
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void oled_show_data_task(void)
{
	if(g_sysconfig.key_page == 0)
	{
		RTC_Get();	 									//获取时间
		if(rtctime!=calendar.sec)						//一秒钟更新一次
		{		
				rtctime=calendar.sec;
				if((calendar.hour<=24)&&(calendar.min<=60)&&(calendar.week<=7)&&(calendar.w_date<=31)&&(calendar.w_month<=12))//检测成功
				{
					sprintf((char *)g_sysconfig.current_timer,(const char *)"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);
					OLED_ShowString(20,0,(unsigned char *)g_sysconfig.current_timer, 16); 
					
					sprintf((char *)g_sysconfig.current_timer2,(const char *)"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
					OLED_ShowString(1,16,(unsigned char *)g_sysconfig.current_timer2, 16); 
					
					switch(calendar.week)
					{
						case 0:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周日");
							break;
						case 1:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周一");
							break;
						case 2:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周二");
							break;
						case 3:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周三");
							break;
						case 4:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周四");
							break;
						case 5:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周五");
							break;
						case 6:
							LCD_ShowChinse_GB162(80,16,(u8 *)"周六");
							break;
					}
				}
		 }
		
		if(g_sysconfig.key_num == 1)
		{
			LCD_ShowChinse_GB162(1,48,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 2)
		{
			LCD_ShowChinse_GB162(25,48,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 3)
		{
			LCD_ShowChinse_GB162(60,48,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 4)
		{
			LCD_ShowChinse_GB162(85,48,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 5)
		{
			g_sysconfig.key_num = 0;
		}
		LCD_ShowChinse_GB162(1,32,(u8 *)"吃药时间");
		
		sprintf(morning_timer,"%02d:%02d",g_sysconfig.mor_hour, g_sysconfig.mor_min);	 
		OLED_ShowString(1,48,(unsigned char *)morning_timer, 16);	 
		
		sprintf(night_timer,"%02d:%02d",g_sysconfig.nig_hour, g_sysconfig.nig_min);	 
		OLED_ShowString(60,48,(unsigned char *)night_timer, 16);	
		
		OLED_Refresh();
	}else if(g_sysconfig.key_page == 1)
	{
		LCD_ShowChinse_GB162(1,0,(u8 *)"药类");
		LCD_ShowChinse_GB162(40,0,(u8 *)"A");
		LCD_ShowChinse_GB162(70,0,(u8 *)"B");
		LCD_ShowChinse_GB162(103,0,(u8 *)"C");
		
		LCD_ShowChinse_GB162(1,16,(u8 *)"药量");
		
		sprintf(g_sysconfig.char_med_A,"%d ",g_sysconfig.med_A);
		OLED_ShowString(40,16,(unsigned char *)g_sysconfig.char_med_A, 16);
		
		sprintf(g_sysconfig.char_med_B,"%d ",g_sysconfig.med_B);
		OLED_ShowString(70,16,(unsigned char *)g_sysconfig.char_med_B, 16);
		
		sprintf(g_sysconfig.char_med_C,"%d ",g_sysconfig.med_C);
		OLED_ShowString(103,16,(unsigned char *)g_sysconfig.char_med_C, 16);
		
		OLED_Refresh();
		
		LCD_ShowChinse_GB162(1,32,(u8 *)"重量");
		LCD_ShowChinse_GB162(45,32,(u8 *)"温度");
		LCD_ShowChinse_GB162(90,32,(u8 *)"湿度");
		
		sprintf(g_sysconfig.char_weight,"%d  ",g_sysconfig.weight);
		OLED_ShowString(5,48,(unsigned char *)g_sysconfig.char_weight, 16);
		
		sprintf(g_sysconfig.char_temp,"%d ",g_sysconfig.temperature);
		OLED_ShowString(45,48,(unsigned char *)g_sysconfig.char_temp, 16);
		
		sprintf(g_sysconfig.char_humi,"%d ",g_sysconfig.humidity);
		OLED_ShowString(90,48,(unsigned char *)g_sysconfig.char_humi, 16);
		
		OLED_Refresh();
		
		if(g_sysconfig.key_num == 1)
		{
			LCD_ShowChinse_GB162(40,16,(u8 *)" ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 2)
		{
			LCD_ShowChinse_GB162(70,16,(u8 *)" ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 3)
		{
			LCD_ShowChinse_GB162(103,16,(u8 *)" ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 4)
		{
//			bring();
			g_sysconfig.key_num = 0;
		}
	}else if(g_sysconfig.key_page == 2)
	{
		if(g_sysconfig.key_num == 1)
		{
			LCD_ShowChinse_GB162(1,0,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 2)
		{
			LCD_ShowChinse_GB162(40,0,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 3)
		{
			LCD_ShowChinse_GB162(63,0,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 4)
		{
			LCD_ShowChinse_GB162(1,16,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 5)
		{
			LCD_ShowChinse_GB162(25,16,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 6)
		{
			LCD_ShowChinse_GB162(50,16,(u8 *)"  ");
			OLED_Refresh();
		}else if(g_sysconfig.key_num == 7)
		{
			g_sysconfig.key_num = 0;
		}
		
		sprintf((char *)g_sysconfig.time_str,(const char *)"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);
		OLED_ShowString(1,0,(unsigned char *)g_sysconfig.time_str, 16); 
		
		sprintf((char *)g_sysconfig.time_str2,(const char *)"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
		OLED_ShowString(1,16,(unsigned char *)g_sysconfig.time_str2, 16); 
		
		OLED_Refresh();
	}
}

/*
************************************************************
*	函数名称：	usart1_receive_data_task
*
*	函数功能：	串口校准时间
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
void usart1_receive_data_task(void)
{
//	char onenet_rx[5] = {0};
	if(s.usart_recv)
	{
		al_debug_log("Tips:     接收到校准时间：%s\r\n",s.usart_buff);
		if(strstr((char *)s.usart_buff,"SET_CCLK"))//SET_CCLK:2026/03/05 17:06:18
		{
	    strncpy(cclk.Year, (char*)s.usart_buff + 9, 4);
			strncpy(cclk.Mon, (char*)s.usart_buff + 14, 2);
			strncpy(cclk.day, (char*)s.usart_buff + 17, 2);
			strncpy(cclk.Hour, (char*)s.usart_buff + 20, 2);
			strncpy(cclk.Min, (char*)s.usart_buff + 23, 2);
			strncpy(cclk.Sec, (char*)s.usart_buff + 26, 2);

			calendar.w_year = atoi(cclk.Year);
			calendar.w_month = atoi(cclk.Mon);
			calendar.w_date = atoi(cclk.day);
			calendar.hour = atoi(cclk.Hour);
			calendar.min = atoi(cclk.Min);
			calendar.sec = atoi(cclk.Sec);
			RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec); //校准时间        
      sprintf(g_sysconfig.timer,"%04d/%02d/%02d %02d:%02d:%02d",calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);		
			al_debug_log("北京时间：%04d/%d/%02d %02d:%02d:%02d\n", calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
		}
		s.usart_recv=0;
		s.usart_index=0;
		memset(s.usart_buff, 0, 1024);
	}
}

