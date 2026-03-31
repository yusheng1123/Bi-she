
/*-----------------------------------------------------*/
/*                     程序结构                        */
/*-----------------------------------------------------*/
/*USER     ：包含程序的main函数，是整个程序的入口      */
/*HARDWARE ：包含开发板各种功能外设的驱动程序          */
/*CORE     ：包含STM32的核心程序，官方提供，我们不修改 */
/*STLIB    ：官方提供的库文件，我们不修改              */
/*-----------------------------------------------------*/
/*                                                     */
/*           程序main函数，入口函数头文件              */
/*                                                     */
/*-----------------------------------------------------*/

#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"
//#include "stm32f10x.h"
#include "led.h"
#include "sys.h"
#include "key.h"
#include "beep.h"
#include "adc.h"
#include "wifi.h"
#include "mqtt.h"
#include "Relay.h"
#include "delay.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "timer1.h"
#include "timer2.h"
#include "timer3.h"
#include "timer4.h"
#include "al_debug.h"
#include "HX711.h"
#include "oled.h"
#include "ds1302.h"
#include "dht11.h"
#include "infrared.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义滞回偏移量（可根据实际场景调整）
#define HYSTERESIS 1    // 滞回值



void LED1_State(void);
void sys_task(void);
void sys_debug_task(void);
void usart1_recv_data_task(void);
void sys_init_show_task(void);
void sys_data_show_task(void);
void sys_publish_data(void);		//往云平台发送数据
void sys_change_publish_data(void);		//往云平台发送更新数据
void usart3_recv_data_task(void);
void sys_polcie_task(void);			//报警任务
void al_rece_asr01_data_task(void);
void key_set_data_task(void);
void oled_show_data_task(void);
void lcd_data_show_task(void);
void usart1_receive_data_task(void);
void system_timer_data_task(void);
#endif











