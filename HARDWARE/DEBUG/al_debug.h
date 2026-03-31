#ifndef AL_DEBUG_H
#define AL_DEBUG_H

#include <stdint.h>
#include "stm32f10x.h"
#include "al_debug.h"
#include "stdio.h"

#define g_version	"General_App_V1.0.0"

#define true 1
#define false 0
	
typedef struct
{
	char Year[5];
	char Mon[5];
	char day[5];
	char Hour[5];
	char Min[5];
	char Sec[5];
}CCLK;

extern CCLK cclk;

typedef struct
{
	uint8_t t,rtc_time_state,open_mor_rtc_state,open_nig_rtc_state,model_state,send_data_flag,hcsr501_state,fan,sg,fan_state,led_state,key_num,key_page,open_eat_state,open_flag,infrared_flag;  
	
	u8 light,rain,temperature,humidity,mor_hour,mor_min,nig_hour,nig_min,weight_state,med_A,med_B,med_C,nor1,nor2,nor3,nor4;
	
	u8 soil_control_state,water_control_state,humi_state,set_mode,humi,open_hcsr501_state,white_led;
	
	int year,month,date,hour,min,sec; 
	
	int mq2_somke,mq2_somke_t,voltage_t,current_t,open_relay_time,mq135_ppm,mq135_ppm_t,mq3_ppm,mq3_ppm_t,init_weight,weight,eat_init_weight,weight_t,alarm_clock;
	
	float soil_temp,soil_humi,vol_mq2_initl,vol_mq2,voltage,current,vol_mq135,mq3_init_ppm,vol_mq3;
	
	double lon,lat;   
	
	uint16_t N_value,P_value,K_value,adcx_mq2,adcx_mq4,adcx_mq135,adcx_mq3;
	
	char char_temp[16],char_humi[16],char_med_A[10],char_med_B[10],char_med_C[10],char_weight[16],char_weight_t[10];
	
	u8 temp_t,humi_t,light_t,rain_t,K_t,N_t,P_t;
	
	u8 light_state;
	
	char char_temp_t[16],char_humi_t[16],char_light_t[16],char_mq3_ppm_t[16],char_P_t[16],char_light[16],char_mq2_somke_t[16],timer_rh[20],timer[50],morning_timer[30];
	char current_timer[30],current_timer2[30],time_str[30],time_str2[30];
	
}al_sysconfig_t;

extern al_sysconfig_t g_sysconfig;


#define al_debug_log(format, ...) u1_printf("<ST>F:"__FILE__"\tL:%d>> "format"<END>\r\n",__LINE__,##__VA_ARGS__)

#ifndef AL_DEBUG_FILE_DEBUG
#define AL_DEBUG_FILE_DEBUG			1
#endif

#define USART1_DEBUG		USART1		//调试打印所使用的串口组
#define USART2_DEBUG		USART2
#define USART3_DEBUG		USART3

void AL_DEBUG_LOG(USART_TypeDef *USARTx, char *fmt,...);
void al_main_task_log_start(void);
void split_data(char *src,const char *separator,char **dest,int *num) ;
uint8_t get_sub_str(char *str, char *separator1, char *separator2, int8_t num, char *substr);
u8 Weather_analysis(u8* buff,u8 *Weather_stat,u8 *data);
u8 Weather_analysis_timer(u8* buff, u8 *Time_stat, u8 *hour_min);
//u8 Weather_analysis(u8* buff, u8 *Weather_stat, u8 *data, u8 is_time); 
#endif



