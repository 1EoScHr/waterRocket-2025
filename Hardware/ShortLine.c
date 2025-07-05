#include "stm32f10x.h"                  // Device header

void ShortLine_Init(void) //短接线初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t ShortLine(void)//短接线不用考虑那么多消抖一类
{
	uint8_t ret=0;
	if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1))
		{
			ret=1;
		}
	return ret;
}
