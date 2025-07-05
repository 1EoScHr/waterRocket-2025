#include "stm32f10x.h"                  // Device header

/*此处为软件SPI，需要时钟线、片选、主机出从机入、主机入从机出*/
/*默认使用模式0*/

//软件SPI速度非常快，不用像I2C一样加延时
void leoSPI_W_SS(uint8_t BitValue)	//写SS的引脚来从机选择，输出
{
			GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);
}

void leoSPI_W_SCK(uint8_t BitValue)	//写SCK的引脚来控制时钟，输出
{
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)BitValue);
}

void leoSPI_W_MOSI(uint8_t BitValue)	//写MOSI的引脚来输出数据，输出
{
			GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)BitValue);
}

uint8_t leoSPI_R_MISO(void)	//读MISO的引脚来读入数据，输入
{
			return GPIO_ReadInputDataBit(GPIOA,	GPIO_Pin_6);
}

void leoSPI_Init(void)	//SPI初始化
{
			//GPIO初始化通信引脚
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
			GPIO_InitTypeDef GPIO_InitStructure;
	
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//输出使用推挽输出模式
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_4;	//时钟、主机出、片选为输出
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//输入使用上拉输入模式
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	//主机入为输入
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
			//初始化后各引脚默认设置
			leoSPI_W_SS(1);	//置高，默认不选中从机
			leoSPI_W_SCK(0);	//计划使用模式0，默认低电平
}

//实现SPI三种默认时序单元
void leoSPI_Start(void)	//起始条件：SS从高电平置低电平
{
			leoSPI_W_SS(0);
}

void leoSPI_Stop(void)	//终止条件：SS从低电平置高电平
{
			leoSPI_W_SS(1);
}

uint8_t leoSPI_SwapByte(uint8_t ByteSend)	//模式0交换字节，用其他模式要对应更改
{
			for (uint8_t i = 0; i < 8; i ++)
			{
				//2.移出数据
				leoSPI_W_MOSI(ByteSend & 0x80);	//从高到低按位移出
				ByteSend <<= 1;
				
				//3.SCK上升沿
				leoSPI_W_SCK(1);
		
				//4.移入数据
				if (leoSPI_R_MISO() == 1)
				ByteSend |= 0x01;	//从高到低按位移入
		
				//1.SS或SCK下降沿
				leoSPI_W_SCK(0);
			}
			
			return ByteSend;
}
