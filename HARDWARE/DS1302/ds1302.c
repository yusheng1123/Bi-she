#include "ds1302.h"
#include "sys.h"
#include "delay.h"
#include "oled.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//*****************DS1302控制命令*******************
#define WRITE_SECOND              0x80
#define WRITE_MINUTE              0x82
#define WRITE_HOUR                0x84
#define WRITE_DAY                 0x86
#define WRITE_MONTH               0x88
#define WRITE_WEEK                0x8A
#define WRITE_YEAR                0x8C
#define WRITE_TIMER_FLAG          0xC0

#define READ_SECOND               0x81
#define READ_MINUTE               0x83
#define READ_HOUR                 0x85
#define READ_DAY                  0x87
#define READ_MONTH                0x89
#define READ_WEEK                 0x8B
#define READ_YEAR                 0x8D
#define READ_TIMER_FLAG           0xC1
#define WRITE_PROTECT             0x8E


_calendar_obj calendar;							//时钟结构体 
_calendar_obj Alarm1={8};	//闹钟结构体
_calendar_obj Alarm2={12};	//闹钟结构体
_calendar_obj Alarm3={20};	//闹钟结构体



_next_obj next;                               		
//月份数据表											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
u16 rtctime;			//时间
char time_str[50];
u8 BCD2HEX(u8 bcd_data)    //BCDtoHEX   
{   
    u8 temp;   
    temp=(bcd_data/16*10 + bcd_data%16);   
    return temp;   
}   
u8 HEX2BCD(u8 hex_data)    //HEXtoBCD    
{   
    u8 temp;   
    temp=(hex_data/10*16 + hex_data%10);   
    return temp;   
}   
//============================================
//函数名称：void Ds1302_Write_Byte (byte addr, byte dat)  
//功能：    串行发送地址、数据，先发低位，且在上升沿发送
//参数传递：有，地址和数据
//返回值：  无
//===========================================
void Ds1302_Write_Byte(u8 addr, u8 dat)     
{
	u8 i;
    DS1302_IO_OUT();                //数据端口定义为输出
    DS1302_RST = 1;
	Delay_Us(10);
	DS1302_CLK = 0;
	Delay_Us(10);
	for(i=0;i<8;i++) 
	{ 
		if(addr&0x01) 
		{
			DS1302_DATA_OUT = 1;
		}
		else 
		{
			DS1302_DATA_OUT = 0;
		}
		addr = addr>>1;
		DS1302_CLK = 1;
		Delay_Us(10);
		DS1302_CLK = 0;
		Delay_Us(10);

	} 
	for(i=0;i<8;i++)  			//写入数据：dat
  {
		if(dat&0x01) 
		{
			DS1302_DATA_OUT = 1;
		}
		else 
		{
			DS1302_DATA_OUT = 0;
		}
		dat = dat>>1;
		DS1302_CLK = 1;
		Delay_Us(10);
		DS1302_CLK = 0;
		Delay_Us(10);
  }
  DS1302_RST = 0;;                     //停止DS1302总线
	Delay_Us(10);
}
//===============================================
//函数名称：byte Ds1302_Read_Byte ( byte addr )
//功能：    串行读取数据，先发低位，且在下降沿发送
//参数传递：有，地址
//返回值：  有，读取的数据
//===============================================
u8 Ds1302_Read_Byte(u8 addr)
{
    u8  i;
    u8  temp = 1;
    DS1302_RST = 1;
    Delay_Us(10);
    for(i=0;i<8;i++) 
    {      
        DS1302_CLK = 0;
        Delay_Us(10);
        if(addr&0x01) 
        {
           DS1302_DATA_OUT = 1;
        }
        else 
        {
           DS1302_DATA_OUT = 0;
        }
        addr = addr>>1;
        DS1302_CLK = 1;
        Delay_Us(10);
    }
    DS1302_IO_IN();                  //数据端口定义为输入
    for(i=0;i<8;i++) 
    {
        temp = temp >> 1;							 //输出数据：temp
        DS1302_CLK = 0;
        Delay_Us(10);
        if(DS1302_DATA_IN) 
        {
            temp |= 0x80;
        }
        DS1302_CLK = 1;
        Delay_Us(10);
    }
    DS1302_IO_OUT();                //数据端口定义为输出                       
    DS1302_CLK = 0;
    Delay_Us(10);
    DS1302_RST = 0;                         //停止DS1302总线
    Delay_Us(10);
    return temp;                    
}
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;	
	yearH=year/100;	
	yearL=year%100; 									//如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;					//所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  
//===============================================
//           向DS1302写入时钟数据
//===============================================
void RTC_Set(u16 year,u8 mon,u8 day,u8 hour,u8 min,u8 sec) 
{    
	u8 WR_week;
	u8 WR_yearL = 0;
	if(year>=2000)
	{
		WR_yearL = year - 2000;
	}
	WR_week = RTC_Get_Week(year,mon,day);						 //根据写入的日期算星期几
	Ds1302_Write_Byte(WRITE_PROTECT,0x00);           //关闭写保护 
	Ds1302_Write_Byte(WRITE_SECOND,0x80);            //暂停 
//	Ds1302_Write_Byte(ds1302_charger_add,0xa9);    //涓流充电 
	Ds1302_Write_Byte(WRITE_YEAR,HEX2BCD(WR_yearL)); //年 
	Ds1302_Write_Byte(WRITE_MONTH,HEX2BCD(mon));     //月 
	Ds1302_Write_Byte(WRITE_DAY,HEX2BCD(day));       //日 
	Ds1302_Write_Byte(WRITE_HOUR,HEX2BCD(hour));     //时 
	Ds1302_Write_Byte(WRITE_MINUTE,HEX2BCD(min));    //分
	Ds1302_Write_Byte(WRITE_SECOND,HEX2BCD(sec));    //秒
	Ds1302_Write_Byte(WRITE_WEEK,HEX2BCD(WR_week));  //周 
	Ds1302_Write_Byte(WRITE_PROTECT,0x80);           //打开写保护 
}

//========================================
//           从DS1302读出时钟数据
//========================================
void RTC_Get(void)  
{ 
//	u8  i,tmp;
	calendar.w_year  = BCD2HEX(Ds1302_Read_Byte(READ_YEAR)); 					//年 
	calendar.w_month = BCD2HEX(Ds1302_Read_Byte(READ_MONTH));         //月 
	calendar.w_date  = BCD2HEX(Ds1302_Read_Byte(READ_DAY));           //日 
	calendar.hour    = BCD2HEX(Ds1302_Read_Byte(READ_HOUR));          //时 
	calendar.min     = BCD2HEX(Ds1302_Read_Byte(READ_MINUTE));        //分 
	calendar.sec     = BCD2HEX(Ds1302_Read_Byte(READ_SECOND)&0x7F); 	//秒 
	calendar.week    = BCD2HEX(Ds1302_Read_Byte(READ_WEEK));          //周 
	calendar.w_year  = calendar.w_year+2000;
}

//==========================================
//              DS1302初始化
//==========================================
void Ds1302_Init(void)
{ 
	IO_Init();					   							//GPIO初始化	
	DS1302_RST = 0;                                    		//RST脚置低
	DS1302_CLK = 0;                                  		//SCK脚置低
	Ds1302_Write_Byte(WRITE_SECOND,0x00);      		//开始  
//	RTC_Set(2026,3,5,17,00,00) ;  //需要设置时间时将注释取消，更改成功之后重新注销
	      //年月日时分秒
}

//*******************以下UTC时间计算部分函数*****************
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) 																//必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;									//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   


void IO_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DS1302_CLK_GPIO_CLOCK|DS1302_DATA_GPIO_CLOCK|DS1302_RST_GPIO_CLOCK, ENABLE);	//使能PA,PB,PC端口时钟
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 													//IO口速度为50MHz	
	//PC端口初始化	
	GPIO_InitStructure.GPIO_Pin = DS1302_CLK_GPIO_PIN;						
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		 												//推挽输出	
	GPIO_Init(DS1302_CLK_GPIO_WHAT, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = DS1302_DATA_GPIO_PIN;					
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		 												//推挽输出	
	GPIO_Init(DS1302_DATA_GPIO_WHAT, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = DS1302_RST_GPIO_PIN;						
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		 												//推挽输出	
	GPIO_Init(DS1302_RST_GPIO_WHAT, &GPIO_InitStructure);	   
}


//void TIME(void)
//{
//	RTC_Get();	 									//获取时间
//	if(rtctime!=calendar.sec)						//一秒钟更新一次
//	{		
//        rtctime=calendar.sec;
//        if((calendar.hour<=24)&&(calendar.min<=60)&&(calendar.week<=7)&&(calendar.w_date<=31)&&(calendar.w_month<=12))//检测成功
//        {
//            sprintf((char *)time_str,(const char *)"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);
////            OLED_ShowText(24,2,(u8 *)time_str,16);
//					  OLED_ShowString(20,0,(unsigned char *)time_str, 16); 
//            
//            sprintf((char *)time_str,(const char *)"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
////            OLED_ShowText(32,4,(u8 *)time_str,16);
//					  OLED_ShowString(1,16,(unsigned char *)time_str, 16); 
//            
//            switch(calendar.week)
//            {
////                case 0:  OLED_ShowText(38,6,(u8*)"星期日",0);break;//星期一
////                case 1:  OLED_ShowText(38,6,(u8*)"星期一",0);break;//星期二
////                case 2:  OLED_ShowText(38,6,(u8*)"星期二",0);break;//星期三
////                case 3:  OLED_ShowText(38,6,(u8*)"星期三",0);break;//星期四
////                case 4:  OLED_ShowText(38,6,(u8*)"星期四",0);break;//星期五
////                case 5:  OLED_ShowText(38,6,(u8*)"星期五",0);break;//星期六
////                case 6:  OLED_ShowText(38,6,(u8*)"星期六",0);break;//星期日
//							

//							case 0:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周日");
//								break;
//							case 1:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周一");
//								break;
//							case 2:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周二");
//								break;
//							case 3:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周三");
//								break;
//							case 4:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周四");
//								break;
//							case 5:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周五");
//								break;
//							case 6:
//								LCD_ShowChinse_GB162(80,16,(u8 *)"周六");
//								break;
//            }
//        }
////        else OLED_ShowText(0,6,(u8 *)"读取失败",0);
//	 }
//}



