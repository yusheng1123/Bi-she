#ifndef __DS1302_H
#define __DS1302_H
#include "sys.h"


//-------------------------------------------------------------------------*
//文件名:  DS1302.h (实时时钟头文件)                                          *
//-------------------------------------------------------------------------*

/*更改引脚之后修改这个地方，刚才是PB0的，现在改成PA5的，只用修改这个地方就可以了*/
#define DS1302_CLK_GPIO_CLOCK     	RCC_APB2Periph_GPIOC    //时钟
#define DS1302_CLK_GPIO_WHAT       	GPIOC					//GPIO组
#define DS1302_CLK_GPIO_PIN        	GPIO_Pin_15				//GPIO脚

#define DS1302_DATA_GPIO_CLOCK      RCC_APB2Periph_GPIOC    //时钟
#define DS1302_DATA_GPIO_WHAT       GPIOC					//GPIO组
#define DS1302_DATA_GPIO_PIN        GPIO_Pin_14				//GPIO脚
#define DS1302_DATA_GPIO_PIN_DATA   14					    //就是第几个

#define DS1302_RST_GPIO_CLOCK       RCC_APB2Periph_GPIOC    //时钟
#define DS1302_RST_GPIO_WHAT       	GPIOC					//GPIO组
#define DS1302_RST_GPIO_PIN        	GPIO_Pin_13				//GPIO脚	

/*更改引脚之后修改这个地方，刚才是PB0的，现在改成PA5的，只用修改这个地方就可以了*/

//IO操作函数			
#define DS1302_CLK 			PCout(15)

#define	DS1302_DATA_IN      PCin(14)  
#define DS1302_DATA_OUT		PCout(14)

#define DS1302_RST 			PCout(13)

//IO方向设置
#if (DS1302_DATA_GPIO_PIN_DATA < 8 )
#define GPIO_DATA (0xFFFFFFFF & (~(0x0f << 4 * DS1302_DATA_GPIO_PIN_DATA)))
#define DS1302_IO_IN()  {DS1302_DATA_GPIO_WHAT->CRL&=GPIO_DATA;DS1302_DATA_GPIO_WHAT->CRL|=8<<4*DS1302_DATA_GPIO_PIN_DATA;}
#define DS1302_IO_OUT() {DS1302_DATA_GPIO_WHAT->CRL&=GPIO_DATA;DS1302_DATA_GPIO_WHAT->CRL|=3<<4*DS1302_DATA_GPIO_PIN_DATA;}//X乘以4=20
#endif

#if (DS1302_DATA_GPIO_PIN_DATA >= 8 )
#define GPIO_DATA (0xFFFFFFFF & (~(0x0f << 4 * (DS1302_DATA_GPIO_PIN_DATA-8))))
#define DS1302_IO_IN()  {DS1302_DATA_GPIO_WHAT->CRH&=GPIO_DATA;DS1302_DATA_GPIO_WHAT->CRH|=8<<4*(DS1302_DATA_GPIO_PIN_DATA-8);}
#define DS1302_IO_OUT() {DS1302_DATA_GPIO_WHAT->CRH&=GPIO_DATA;DS1302_DATA_GPIO_WHAT->CRH|=3<<4*(DS1302_DATA_GPIO_PIN_DATA-8);}//X乘以4=20
#endif


typedef struct 
{
    u8  sec;
    u8  min;
    u8  hour;
    u8  day;
    u8  mon;
    u16 year;
    u8  week;
}_next_obj;	
extern _next_obj next;
typedef struct 
{
	vu8 hour;
	vu8 min;
	vu8 sec;			
	//公历日月年周
	vu16 w_year;
	vu8  w_month;
	vu8  w_date;
	vu8  week;		 
}_calendar_obj;	

extern _calendar_obj calendar;	//日历结构体
extern _calendar_obj Alarm1;	//闹钟结构体
extern _calendar_obj Alarm2;	//闹钟结构体
extern _calendar_obj Alarm3;	//闹钟结构体

extern u16 rtctime;
extern u32 RTC_sec_sum;														//当前时间的总秒值
extern u32 Program_sec_sum;												//当前编程任务的总秒值,与RTC_sec_sum进行比较
void RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec);
void RTC_Get(void);
void NEXT_Date(u8 day);
void IO_Init(void);
void TIME(void);      //显示时间
u8 RTC_Pro_count(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec,u8 mode);//编程任务时间计算
u8 Pro_Get_time(u32 ttt);								//编程模式无效时间时计算下次开始的日期
void Ds1302_Init(void);

#endif
