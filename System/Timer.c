#include "stm32f10x.h"

/*定时器实际就是计数器！！！*/

void Timer_Internal_Init(void)
{
			//先开启TIM对应的RCC内部时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
			//选择时基单元的时钟，不选择默认内部
			//TIM_InternalClockConfig				//RCC内部时钟
			//TIM_ITRxExternalClockConfig		//ITRx其他定时器
			//TIM_TIxExternalClockConfig		//TIx捕获通道
			//TIM_ETRClockMode1Config
			//TIM_ETRClockMode2Config				//EXT外部时钟
			
			//选择内部时钟驱动TIM的时基单元
			TIM_InternalClockConfig(TIM2);
	
			//配置时基单元
			TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	//定义相关结构体
			TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//时钟分频，与滤波相关，选择1分频
			TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，向上向下中央对齐等，选择向上对齐
			TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	//重复计数器的值，高级定时器才有，置零
			
	//关键参数：定时频率=72M/(PSC+1)/(ARR+1)，均在0-65535间
			TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;	//ARR自动重装器的值
			TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;	//PSC预分频器的值
	//这里得到0.1s的分度，更精细可以修改
			TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
			//手动清除标志位，避免刚初始化完就中断使得计时从0开始
			TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	
			//使能更新中断(Update)到NVIC的通路
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
			
			//NVIC优先级分组
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			
			//配置NVIC中断
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	//中断通道,选择TIM对应的
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//抢占优先级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//响应优先级
			NVIC_Init(&NVIC_InitStructure);
			
			//使能，定时器定时中断
			TIM_Cmd(TIM2, ENABLE);
}

void Timer_External_Init(void)	//外部中断定时器(计数器)
{
			//在内部中断的模板之上修改即可，重复的注释删掉
	
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
			//外部要定义GPIO引脚
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//推荐浮空，但会跳，选择上拉
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//TIM的EXT引脚复用
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
			TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0x0F);
	
			TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
			TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	
			TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	
			TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
			
	//关键参数：定时频率=72M/(PSC+1)/(ARR+1)，均在0-65535间
	//外部触发以对射红外为例，预分频为挡几次CNT加一
	//												自重装为CNT到多少才中断一次
			TIM_TimeBaseInitStructure.TIM_Period = 10 - 1;	//ARR自动重装器的值
			TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;	//PSC预分频器的值

			TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
			TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	
			TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
			
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_Init(&NVIC_InitStructure);
			
			TIM_Cmd(TIM2, ENABLE);
}

/*定时中断直接复制在需要使用的地方即可
void TIM2_IRQHandler(void)	//选择TIM的中断处理函数，产生更新中断时会自动执行
{
			
			if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	//先检查中断标志位，获取TIM的更新中断标志位
			{
					//填写相应的函数
					TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//清除标志位
			}
}
*/





