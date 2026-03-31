
/*                                                 */
/*              操作Wifi功能的头文件               */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __WIFI_H
#define __WIFI_H

#include "usart2.h"	    //包含需要的头文件
#include "stm32f10x.h"

#define WIFI_NAME "csk"//WIFI名
#define WIFI_PASSWORD "12345678"//wifi密码

#define SERVER_IP "sh-5-mqtt.iot-api.com"//服务器IP
#define SERVER_PORT 1883 //端口号

extern char morning_timer[];
extern char night_timer[];

struct ESP8266
{
	u8 esp8266_name[20];//WIFI名称
	u8 esp8266_key[20];//WIFI密码
	u8 esp8266_ip[20];//IP地址
	u16 esp8266_prot;//端口号
};
extern struct ESP8266 esp8266_info;
u8 Esp8266_AP_TcpServer_Init(u8 *wifi_name,u8 *wifi_password,u16 port);
u8 Esp8266_SendCmdCheckStat(char *cmd,char *data);
u8 ESP8266_GetConnectStat(void);
u8 Esp8266_SendData(u8 clinet_prot,u8 *data);
u8 Esp8266_STA_TCPclinet_Init(u8 *wifi_name,u8 *wifi_key,u8 *server_ip,u16 server_port);
u8 ESP8266_ConectWifi(u8 *name,u8 *key);
void WiFi_Connect_IoTServer(void);
void sys_reve_thingscloud_data(void);
#endif


