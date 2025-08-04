#include "stm32f10x.h"                  // Device header

/*此处为软件SPI，需要时钟线、片选、主机出从机入、主机入从机出*/
/*默认使用模式0*/

#define leoSPI_SS_GPIO 			GPIOA
#define leoSPI_SS_PIN				GPIO_Pin_5
#define leoSPI_SCL_GPIO 		GPIOA
#define leoSPI_SCL_PIN 			GPIO_Pin_7
#define leoSPI_MOSI_GPIO 		GPIOA
#define leoSPI_MOSI_PIN			GPIO_Pin_6
#define leoSPI_MISO_GPIO 		GPIOA
#define leoSPI_MISO_PIN			GPIO_Pin_4
#define leoSPI_APBCLK				RCC_APB2Periph_GPIOA




//软件SPI速度非常快，不用像I2C一样加延时
void leoSPI_W_SS(uint8_t BitValue)	//写SS的引脚来从机选择，输出
{
			GPIO_WriteBit(leoSPI_SS_GPIO, leoSPI_SS_PIN, (BitAction)BitValue);
}

void leoSPI_W_SCK(uint8_t BitValue)	//写SCK的引脚来控制时钟，输出
{
			GPIO_WriteBit(leoSPI_SCL_GPIO, leoSPI_SCL_PIN, (BitAction)BitValue);
}

void leoSPI_W_MOSI(uint8_t BitValue)	//写MOSI的引脚来输出数据，输出
{
			GPIO_WriteBit(leoSPI_MOSI_GPIO, leoSPI_MOSI_PIN, (BitAction)BitValue);
}

uint8_t leoSPI_R_MISO(void)	//读MISO的引脚来读入数据，输入
{
			return GPIO_ReadInputDataBit(leoSPI_MISO_GPIO, leoSPI_MISO_PIN);
}

void leoSPI_Init(void)	//SPI初始化
{
			//GPIO初始化通信引脚
			RCC_APB2PeriphClockCmd(leoSPI_APBCLK, ENABLE);	
			GPIO_InitTypeDef GPIO_InitStructure;
	
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//输出使用推挽输出模式
			GPIO_InitStructure.GPIO_Pin = leoSPI_SCL_PIN | leoSPI_MOSI_PIN | leoSPI_SS_PIN;	//时钟、主机出、片选为输出
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(leoSPI_SS_GPIO, &GPIO_InitStructure);
	
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//输入使用上拉输入模式
			GPIO_InitStructure.GPIO_Pin = leoSPI_MISO_PIN;	//主机入为输入
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(leoSPI_MISO_GPIO, &GPIO_InitStructure);
	
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
