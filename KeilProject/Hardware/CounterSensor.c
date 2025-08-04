#include "stm32f10x.h"                  // Device header

/*对射式红外传感计次，主要用于外部中断的学习*/

uint16_t CounterSensor_Count;

void CounterSensor_Init(void)
{
			//打开对应外设时钟
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//GPIO外设
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	//AFIO外设，负责重定义与外部中断
			//NVIC内核外设不需要开启时钟
			//EXTI未知
	
			//配置GPIO
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			
			//配置AFIO
			GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	
			//配置EXTI
			EXTI_InitTypeDef EXTI_InitStructure;
			EXTI_InitStructure.EXTI_Line = EXTI_Line14;	//选择的引脚对应中断线
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;	//开启，选择ENABLE
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	//中断or事件
			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	//边沿触发
			EXTI_Init(&EXTI_InitStructure);
	
			//配置NVIC
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//NVIC中断优先级分配，一套里面只有一个
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	//指定中断通道，根据中断线对应
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//响应优先级
			NVIC_Init(&NVIC_InitStructure);	
}

uint16_t CounterSensor_Get(void)
{
			return CounterSensor_Count;
}

void EXTI15_10_IRQHandler(void)	//中断处理函数名是固定的
{
			//10-15都能进来，先判断是不是需要的那条线
			if (EXTI_GetITStatus(EXTI_Line14) == SET)
			{
					/*填写处理函数*/
					CounterSensor_Count ++;
					EXTI_ClearITPendingBit(EXTI_Line14);
			}
}
