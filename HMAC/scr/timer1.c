
#include "stm32f10x.h"  //包含需要的头文件
#include "timer1.h"
#include "al_debug.h"

u8 check_time = 0;
u8 tft_reflash_falg = 0,tft_reflash_time = 0;

volatile unsigned long    g_Timer1[MAX_TIMER1];

/*-------------------------------------------------*/
/*函数名：定时器1使能1分钟定时                     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM1_ENABLE_60S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;            //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                          //定义一个设置中断的变量
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);           //使能TIM1时钟	
	
  TIM_DeInit(TIM1);                                             //定时器1寄存器恢复默认值
	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	          //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=36000-1;              //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=2-1;          //设置定时器重复计数2次（TIM1和8 才有这个功能）
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;     //1分频
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);            //设置TIM1
	
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);                      //使能TIM1溢出中断    
                       	
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;              //设置TIM1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;       //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;              //子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                               //设置中断
	
	TIM_Cmd(TIM1,ENABLE);                                         //开TIM1   
}


//TIM1_Init(2000-1, 36000-1); // 定时周期1s
void TIM1_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    /* 定时器TIM1初始化 */
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);

    /* 中断使能 */
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    /* 中断优先级NVIC设置 */
    NVIC_InitStructure.NVIC_IRQChannel =  TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM1, ENABLE);

// 全局定时器初始化
    for(int i = 0; i < MAX_TIMER1; i++)
    {
        g_Timer1[i] = 0;
    }
}

/********************************************************************************************************
** 函数: TIM1_IRQHandler,  定时器1中断服务程序
**------------------------------------------------------------------------------------------------------
** 参数: 无
** 返回: 无
********************************************************************************************************/
void TIM1_UP_IRQHandler(void)   //TIM1中断
{
    uint8_t i;

    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)   // 检查TIM1更新中断发生与否
    {
        //-------------------------------------------------------------------------------
        // 各种定时间器计时
        for(i = 0; i < MAX_TIMER1; i++)      // 定时时间递减
            if(g_Timer1[i]) g_Timer1[i]-- ;
				
				
				if(tft_reflash_falg == true)
				{
					tft_reflash_time ++;
				}
				else
					tft_reflash_time = 0;
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);     //清除TIMx更新中断标志
    }
}

