#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/*PWM驱动直流电机：需要连接驱动芯片，要有两个GPIO口控制方向与一个PWM口控制速度*/
/*PWM参数：ARR+1=100，PSC+1=36(增大频率至人耳范围外)，CCR在0-100控制转速*/

void Motor_Init(void)
{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//GPIO初始化，控制方向
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			
			PWM_Init();	//PWM初始化，控速
}

void Motor_SetSpeed(int8_t Speed)	//设置速度，-100到+100
{
			if (Speed >= 0)
			{
					GPIO_SetBits(GPIOA, GPIO_Pin_4);
					GPIO_ResetBits(GPIOA, GPIO_Pin_5);
					PWM_SetCompare1(Speed);
			}
			
			else
			{
					GPIO_SetBits(GPIOA, GPIO_Pin_5);
					GPIO_ResetBits(GPIOA, GPIO_Pin_4);
					PWM_SetCompare1(-1 * Speed);
			}
}


