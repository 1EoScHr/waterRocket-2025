#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/*有源蜂鸣器，PWM频率即频率*/

void Beep_Init(void)
{			
			/*若需要引脚重映射，使用下方函数*/
//			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//打开AFIO
//			//若目标端口上电默认有调试端口，要先取消
//			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
//			GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);	//查表寻找重映射对应参数
	
	
	
			//配置GPIO
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽，因为将控制权转交给复用的PWM功能
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			
			//先开启TIM对应的RCC内部时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
			//选择时基单元的时钟，不选择默认内部
			//有RCC内部时钟、ITRx其他定时器、TIx捕获通道、EXT外部时钟
			//选择内部时钟驱动TIM的时基单元
			TIM_InternalClockConfig(TIM4);
	
			//配置时基单元
			TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	//定义相关结构体
			
			TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//时钟分频，与滤波相关，选择1分频
			TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，向上向下中央对齐等，选择向上对齐
			TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	//重复计数器的值，高级定时器才有，置零
	//关键参数：定时频率=72M/(PSC+1)/(ARR+1)，均在0-65535间
			TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;	//ARR自动重装器的值
			TIM_TimeBaseInitStructure.TIM_Prescaler = 1377 - 1;	//PSC预分频器的值
			TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	
			//配置输出比较
			TIM_OCInitTypeDef TIM_OCInitStructure;
			TIM_OCStructInit(&TIM_OCInitStructure);	//没有给所有成员赋值，先默认初始化
			
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	//输出比较模式
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	//输出比较的极性
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//输出使能
			TIM_OCInitStructure.TIM_Pulse = 50;	//设置CCR
	//计算公式：PWM频率=72M/(PSC+1)/(ARR+1)
	//					占空比=CCR/(ARR+1)
	//					分辨率=1/(ARR+1)
	
			//TIM_OutputNState、TIM_OCIdleState、TIM_OCNIdleState、TIM_OCNPolarity//这些高级定时器才会使用到
			
			TIM_OC2Init(TIM4, &TIM_OCInitStructure);	//根据TIM的CH来配置
			/*同一TIM的多个CH可以继续加*/
			
			//TIM_CtrlPWMOutputs//仅高级定时器使用，在使用高级定时器输出PWM时需调用并使能才能输出
			
			//使能，开启
			TIM_Cmd(TIM4, ENABLE);
}

void Beep_Ring(uint16_t PSC)	//在程序运行中改变PSC，改变频率即音调
{
			TIM_SetCompare2(TIM4, 50);
			TIM_PrescalerConfig(TIM4, PSC, TIM_PSCReloadMode_Update);
}

void Beep_Stop(void)	//停止
{
			TIM_SetCompare2(TIM4, 0);
}

void Beep_JustLoud(void)	//共振响度最大，2700Hz
{
			TIM_SetCompare2(TIM4, 50);
			TIM_PrescalerConfig(TIM4, 266, TIM_PSCReloadMode_Update);
}


/*存放音调*/
//uint16_t Beep_C_low[7] = {};
const uint16_t Beep_C_mid[8] = {1377, 1227, 1093, 1032, 918, 818, 729, 688};
//uint16_t Beep_C_Hih[7] = {};









