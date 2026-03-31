
/*-------------------------------------------------*/
/*                                                 */
/*            操作Wifi功能的源文件              */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "wifi.h"	    //包含需要的头文件
#include "delay.h"	    //包含需要的头文件
#include "usart1.h"	    //包含需要的头文件
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "al_debug.h"
#include "USART2.h"
#include "Relay.h"
#include "beep.h"
#include "timer3.h"
#include "oled.h"
#include "key.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8 idex=0;
int i=0;
char buffer[100];
u8 data[100];
u8 data_buf[32] = {0};
char morning_timer[10]="00:00";
char night_timer[6]="00:00";

struct ESP8266 esp8266_info;
u8 stat=0;
u8 Esp8266_AP_TcpServer_Init(u8 *wifi_name,u8 *wifi_password,u16 port)
{
	char buff[100];
	char *p=NULL;
	u8 i=0;
	u1_printf("1.发送测试指令\r\n");
	if(Esp8266_SendCmdCheckStat("AT\r\n","OK\r\n"))return 1;
	u1_printf("2.关回显\r\n");
	if(Esp8266_SendCmdCheckStat("ATE0\r\n","OK\r\n"))return 2;
	u1_printf("3.设置模式\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CWMODE=2\r\n","OK\r\n"))return 3;	
	u1_printf("4.模块复位\r\n");
	if(Esp8266_SendCmdCheckStat("AT+RST\r\n","OK\r\n"))return 4;	
	Delay_Ms(1000);
	Delay_Ms(1000);
	u1_printf("5.关回显\r\n");
	if(Esp8266_SendCmdCheckStat("ATE0\r\n","OK\r\n"))return 5;	
	//字符串拼接
	snprintf(buff,sizeof(buff),"AT+CWSAP=\"%s\",\"%s\",1,4\r\n",wifi_name,wifi_password);
	u1_printf("buff:%s",buff);
	u1_printf("6.设置热点信息\r\n");
	if(Esp8266_SendCmdCheckStat(buff,"OK\r\n"))return 6;	
	u1_printf("7.设置多连接\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIPMUX=1\r\n","OK\r\n"))return 7;	
	u1_printf("8.设置端口号\r\n");
	snprintf(buff,sizeof(buff),"AT+CIPSERVER=1,%d\r\n",port);
	if(Esp8266_SendCmdCheckStat(buff,"OK\r\n"))return 8;	
	u1_printf("9.查询IP地址\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIFSR\r\n","OK\r\n"))return 9;
	/*****提取IP地址信息*/
	/*USART2:+CIFSR:APIP,"192.168.4.1"
		+CIFSR:APMAC,"86:f3:eb:a6:be:f9"
		OK
	*/
	p=strstr((char *)usart2_arrary,"APIP");
	if(p!=NULL)
	{
		p+=6;
		while((*p!='"') && (*p!='\0'))
		{
			buff[i]=*p;
			p++;
			i++;
		}
	}
	buff[i]='\0';
	u1_printf("buff:%s\r\n",buff);
	strcpy((char *)esp8266_info.esp8266_ip,buff);//IP地址
	strcpy((char *)esp8266_info.esp8266_name,(char *)wifi_name);//wifi名称
	strcpy((char *)esp8266_info.esp8266_key,(char *)wifi_password);//wifi密码
	esp8266_info.esp8266_prot=port;
	return 0;
}
/***************发送指令检测返回值函数*************
**形参：char *cmd -- 要发送的指令
**			char *data  --要检测是返回值
**返回值：0 -- 成功 ，1--失败
**例：Esp8266_SendCmdCheckStat("AT\r\n","OK\r\n")
**
*****************************************************/
u8 Esp8266_SendCmdCheckStat(char *cmd,char *data)
{
	u8 i,j;
	for(i=0;i<5;i++)
	{
		//清除标志位和数组下班
		usart2_index=0;
		usart2_data=0;
		Usartx_SendString(USART2,(u8 *)cmd);//发送指令
		for(j=0;j<100;j++)//循环接收数据
		{
			if(usart2_data)
			{
				usart2_arrary[usart2_index]='\0';
				if(strstr((char *)usart2_arrary,data))
				{
					usart2_index=0;
					usart2_data=0;
					return 0;
				}
				else
				{
					usart2_index=0;
					usart2_data=0;
					memset(usart2_arrary,0,sizeof(usart2_arrary));//清除缓冲区
				}
			}
			Delay_Ms(5);
		}
	}
	return 1;
}
/****************等待客户端连接****************/
u8 ESP8266_GetConnectStat(void)
{
	if(usart2_data)
	{
		usart2_arrary[usart2_index]='\0';
		if(strstr((char *)usart2_arrary,"CONNECT"))
		{
			usart2_data=0;
			usart2_index=0;
			return 1;
		}
		else if(strstr((char *)usart2_arrary,"CLOSED"))
		{
			usart2_data=0;
			usart2_index=0;
			return 2;
		}
		else
		{
			usart2_data=0;
			usart2_index=0;
		}
			
	}
	return 0;
}
/****************TCP服务器发送数据函数****************
**功能;服务器给客户端发送数据
**
**形参：u8 clinet_prot -- 哪个客户端
**			u8 *data       --要发送的数据
**返回值：0 -- 成功，1 -- 失败
**AT+CIPSEND=0,10\r\n
******************************************************/
u8 Esp8266_SendData(u8 clinet_prot,u8 *data)
{
	char buff[20];
	u16 len=strlen((char *)data);//要发送的字符串长度
	snprintf(buff,sizeof(buff),"AT+CIPSEND=%d,%d\r\n",clinet_prot,len);
	if(Esp8266_SendCmdCheckStat(buff,">"))return 1;//没有获取">"
	if(Esp8266_SendCmdCheckStat((char *)data,"SEND OK"))return 2;//数据发送失败
	return 0;
}

u8 Esp8266_STA_TCPclinet_Init(u8 *wifi_name,u8 *wifi_key,u8 *server_ip,u16 server_port)
{
	char buff[100];
	/*退出透传模式*/
	u8 i=0;
	char *p=NULL;
	for(i=0;i<5;i++)
	{
		Usartx_SendString(USART2,(u8 *)"+++");//退出透传模式
		Delay_Ms(500);
		if(Esp8266_SendCmdCheckStat("AT\r\n","OK\r\n")==0)
		{
			i=0;
			break;
		}
	}
	if(i!=0)
	{
		u1_printf("退出透传模式失败\r\n");
		return 0xff;
	}
	u1_printf("1.发送测试指令\r\n");
	if(Esp8266_SendCmdCheckStat("AT\r\n","OK\r\n"))return 1;
	u1_printf("2.关回显\r\n");
	if(Esp8266_SendCmdCheckStat("ATE0\r\n","OK\r\n"))return 2;
	u1_printf("3.设置模式\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CWMODE=1\r\n","OK\r\n"))return 3;	
	u1_printf("4.模块复位\r\n");
	if(Esp8266_SendCmdCheckStat("AT+RST\r\n","OK\r\n"))return 4;	
	Delay_Ms(1000);
	Delay_Ms(1000);
	u1_printf("5.关回显\r\n");
	if(Esp8266_SendCmdCheckStat("ATE0\r\n","OK\r\n"))return 5;	
	

	u1_printf("6.连接WIFI\r\n");
	if(ESP8266_ConectWifi(wifi_name,wifi_key))return 6;
	u1_printf("7.设置单连接\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIPMUX=0\r\n","OK"))return 7;		
	u1_printf("8.查询IP地址\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIFSR\r\n","OK\r\n"))return 8;
	u1_printf("ip:%s\r\n",usart2_arrary);//打印IP信息
	/*
			ip:+CIFSR:STAIP,"192.168.43.111"
			+CIFSR:STAMAC,"84:f3:eb:a6:be:f9"
		OK
	*/
	i=0;
	p=strstr((char *)usart2_arrary,"STAIP,\"");//查找字符串
	if(p!=NULL)
	{
		p+=7;
		while((*p!='"') && (*p!='\0'))
		{
			buff[i++]=*p++;
		}
	}
	buff[i]='\0';
	u1_printf("ip:%s\r\n",buff);
	snprintf(buff,sizeof(buff),"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",server_ip,server_port);
	u1_printf("9.连接服务器\r\n");
	if(Esp8266_SendCmdCheckStat(buff,"OK"))return 9;	
	u1_printf("配置透传模式\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIPMODE=1\r\n","OK\r\n"))return 10;	
	u1_printf("开始发送数据\r\n");
	if(Esp8266_SendCmdCheckStat("AT+CIPSEND\r\n",">"))return 11;	
	return 0;
}
/************************连接WIFI************************
**形参：u8 *name -- 要连接的wifi名称
**			u8 *key  -- wifi密码
**返回值：0 -- 成功，1 -- 失败
***********************************************************/
u8 ESP8266_ConectWifi(u8 *name,u8 *key)
{
	u8 buff[100];
	u8 i,j;
	snprintf((char *)buff,sizeof(buff),"AT+CWJAP=\"%s\",\"%s\"\r\n",name,key);//字符串拼接
//	u1_printf("buff:%s",buff);
	for(i=0;i<5;i++)
	{
		usart2_data=0;
		usart2_index=0;
		Usartx_SendString(USART2,(u8*)buff);//发送连接WIFI指令
		for(j=0;j<100;j++)
		{
			Delay_Ms(50);
			if(usart2_data)
			{
				usart2_arrary[usart2_index]='\0';
//				u1_printf("USART2:%s\r\n",usart2_arrary);
				if(strstr((char *)usart2_arrary,"WIFI GOT IP"))
				{
					usart2_data=0;
					usart2_index=0;
					return 0;
				}
				else
				{
					usart2_data=0;
					usart2_index=0;
					memset(usart2_arrary,0,sizeof(usart2_arrary));
				}
			}
			
		}
	}
	return 1;
}

void WiFi_Connect_IoTServer(void)
{
	while(1)
	{
		stat=Esp8266_STA_TCPclinet_Init((u8 *)WIFI_NAME,(u8 *)WIFI_PASSWORD,(u8 *)SERVER_IP,SERVER_PORT);
		if(stat==0)break;
		Delay_Ms(500);
//		u1_printf("stat=%d\r\n",stat);
	}
	u1_printf("服务器连接成功\r\n");
	while(1)
	{
		MQTT_Init();
		stat=MQTT_Connect(Client_ID,User_name,Pass_word);
		if(stat==0)break;
		Delay_Ms(500);
		u1_printf("正在连接....\r\n");
	}
	u1_printf("连接成功\r\n");
	stat=MQTT_SubscribeTopic(SET_TOPIC,0,1);
	if(stat)u1_printf("订阅失败\r\n");
	else u1_printf("订阅成功\r\n");
}	

void sys_reve_thingscloud_data(void)//接收云平台下发的数据
{
	u8 hour_min[2] = {0};  // 用于接收小时[0]和分钟[1]
	u8 hour_min2[2] = {0};  // 用于接收小时[0]和分钟[1]
	if(usart2_data)
	{
		idex=0;
		for(i=0;i<usart2_index;i++)
		{
//				al_debug_log("%c",usart2_arrary[i]);
			if(usart2_arrary[i]!='\0')	buffer[idex++]=usart2_arrary[i];
		}
		
		buffer[idex]='\0';
		al_debug_log("%s",buffer);
		
		if(Weather_analysis((u8*)buffer,(u8 *)"weight_t",data)==0)
		{
			g_sysconfig.weight_t = atoi((const char *)data);	
			al_debug_log("重量阈值：%d",g_sysconfig.weight_t);				
		}else if(Weather_analysis((u8*)buffer,(u8 *)"temp_t",data)==0)
		{
			g_sysconfig.temp_t = atoi((const char *)data);	
			al_debug_log("温度阈值：%d",g_sysconfig.temp_t);				
		}else if(Weather_analysis((u8*)buffer,(u8 *)"humi_t",data)==0)
		{
			g_sysconfig.humi_t = atoi((const char *)data);	
			al_debug_log("湿度阈值：%d",g_sysconfig.humi_t);				
		}else if(Weather_analysis((u8*)buffer,(u8 *)"med_A",data)==0)
		{
			g_sysconfig.med_A = atoi((const char *)data);	
			al_debug_log("药类A阈值：%d",g_sysconfig.med_A);				
		}else if(Weather_analysis((u8*)buffer,(u8 *)"med_B",data)==0)
		{
			g_sysconfig.med_B = atoi((const char *)data);	
			al_debug_log("药类B阈值：%d",g_sysconfig.med_B);				
		}else if(Weather_analysis((u8*)buffer,(u8 *)"med_C",data)==0)
		{
			g_sysconfig.med_C = atoi((const char *)data);	
			al_debug_log("药类C阈值：%d",g_sysconfig.med_C);				
		}
		
		if (Weather_analysis_timer((u8*)buffer, (u8 *)"mor_timer", hour_min) == 0)
		{
			// 赋值到系统配置变量
       g_sysconfig.mor_hour = hour_min[0];
       g_sysconfig.mor_min = hour_min[1];
			al_debug_log("上午时间解析成功：小时=%d，分钟=%d", g_sysconfig.mor_hour, g_sysconfig.mor_min);
		}else if (Weather_analysis_timer((u8*)buffer, (u8 *)"nig_timer", hour_min2) == 0)
		{
			// 赋值到系统配置变量
       g_sysconfig.nig_hour = hour_min2[0];
       g_sysconfig.nig_min = hour_min2[1];
			al_debug_log("下午时间解析成功：小时=%d，分钟=%d", g_sysconfig.nig_hour, g_sysconfig.nig_min);
		}
				
		if(Weather_analysis((u8*)buffer,(u8 *)"open_mor_rtc_state",data)==0)
		{
			if(strstr((char *)data,"1"))
			{
				g_sysconfig.open_mor_rtc_state = 1;
				LCD_ShowChinse_GB162(98,32,(u8 *)"OK");
				OLED_Refresh();
				bring();
				kk.k8_state = true;
				al_debug_log("上午提醒吃药按钮设置。。。。。。。。。。。。");
			}else if(strstr((char *)data,"0"))
			{
				g_sysconfig.open_mor_rtc_state = 0;
				LCD_ShowChinse_GB162(98,32,(u8 *)"  ");
				OLED_Refresh();
				kk.k8_state = false;
				al_debug_log("上午提醒吃药按钮取消。。。。。。。。。。。。");
			}
		}else	if(Weather_analysis((u8*)buffer,(u8 *)"open_nig_rtc_state",data)==0)
		{
			if(strstr((char *)data,"1"))
			{
				g_sysconfig.open_nig_rtc_state = 1;
				LCD_ShowChinse_GB162(98,32,(u8 *)"OK");
				OLED_Refresh();
				bring();
				kk.k8_state = true;
				al_debug_log("下午提醒吃药按钮设置。。。。。。。。。。。。");
			}else if(strstr((char *)data,"0"))
			{
				g_sysconfig.open_nig_rtc_state = 0;
				LCD_ShowChinse_GB162(98,32,(u8 *)"  ");
				OLED_Refresh();
				kk.k8_state = false;
				al_debug_log("下午提醒吃药按钮取消。。。。。。。。。。。。");
			}
		}
		usart2_data = 0;
		usart2_index = 0;
	}
}

