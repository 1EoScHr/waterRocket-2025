#include "stm32f10x.h"
#include "Delay.h"


/*此处为软件I2C，无需调用I2C库函数，只需用GPIO即可实现I2C
	需要修改对应的时钟线与数据线只需更改前四个函数
	若同时用到了多个使用I2C通信的设备
	可以分别写入各硬件的驱动函数里，也可以连在同一组I2C线上，为I2C的真正运用*/
	
#define leoI2C_SCL_GPIO GPIOB
#define leoI2C_SCL_PIN GPIO_Pin_8
#define leoI2C_SDA_GPIO GPIOB
#define leoI2C_SDA_PIN GPIO_Pin_9


//对读写进行函数封装
void leoI2C_W_SCL(uint8_t BitValue)	//写时钟线
{
			GPIO_WriteBit(leoI2C_SCL_GPIO, leoI2C_SCL_PIN,	(BitAction)BitValue);
			Delay_us(1);//方便高频移植
}

void leoI2C_W_SDA(uint8_t BitValue)	//写数据线
{
			GPIO_WriteBit(leoI2C_SDA_GPIO, leoI2C_SDA_PIN,	(BitAction)BitValue);
			Delay_us(1);
}

uint8_t	leoI2C_R_SDA(void)	//读数据线
{
			uint8_t BitValue;
			BitValue = GPIO_ReadInputDataBit(leoI2C_SDA_GPIO, leoI2C_SDA_PIN);
			Delay_us(1);
			return BitValue;
}

void leoI2C_Init(void)	//I2C初始化
{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //把SCL和SDA初始化为开漏输出模式
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
			GPIO_InitStructure.GPIO_Pin = leoI2C_SCL_PIN;
			GPIO_Init(leoI2C_SCL_GPIO, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = leoI2C_SDA_PIN;
			GPIO_Init(leoI2C_SDA_GPIO, &GPIO_InitStructure);
	
			GPIO_SetBits(leoI2C_SCL_GPIO, leoI2C_SCL_PIN);
			GPIO_SetBits(leoI2C_SDA_GPIO, leoI2C_SDA_PIN);
}

//把I2C对应的6种时序基本单元对应
//SCL实质上是读写分离的表现，SCL为低时，往数据线里写，SCL为高时，从数据线里读
//当高电平时数据被修改，则特异性的为起始与终止

void leoI2C_Start(void)
{
			//起始条件：SCL与SDA高电平时，SDA从高电平切换到低电平,SCL再变为低电平
			leoI2C_W_SDA(1);//先释放SDA比较保险，为兼容重复起始条件，即SCL为低电平，SDA不确定，
											//当SDA也为低时，先释放SCL，再释放SDA为终止条件，导致混淆
	
			leoI2C_W_SCL(1);
			leoI2C_W_SDA(0);
			leoI2C_W_SCL(0);
}

void leoI2C_Stop(void)
{
			//终止条件：SCL与SDA低电平时，SCL从低电平切换到高电平,SDA再变为高电平
			leoI2C_W_SDA(0);//先拉低SDA比较保险，能确保产生上升沿
			leoI2C_W_SCL(1);
			leoI2C_W_SDA(1);
}

void leoI2C_SendByte(uint8_t Byte)
{
			//发送一个字节：SCL低电平期间，主机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，从机从SDA读取数据，主机再次拉低SCL，重复8次
			
			/*除结束标志外，SCL都以0结束，所以开始不用设SCL为0*/
			
			for(uint8_t i = 0; i < 8; i ++)
			{
				leoI2C_W_SDA(Byte & (0x80 >> i));
				leoI2C_W_SCL(1);
				leoI2C_W_SCL(0);	//不必担心跟不上，函数加了延时
			}
}

uint8_t leoI2C_ReceiveByte(void)
{
			//接收一个字节：SCL低电平期间，从机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，主机从SDA读取数据，主机再次拉低SCL，重复8次
			//							由于需要接收，主机每次读取前要释放SDA，将控制权交给从机
			
			uint8_t Byte = 0x00;
	
			leoI2C_W_SDA(1);
			for(uint8_t i = 0; i < 8; i ++)
			{
				
				leoI2C_W_SCL(1);
				if(leoI2C_R_SDA()) Byte |= (0x80 >> i);
				leoI2C_W_SCL(0);
			}
			
			return Byte;
}

void leoI2C_SendAck(uint8_t AckBit)
{
			//发送应答：主机接收完一个字节后，在下一个时钟发送一位数据
			//					0表示收到，1表示未收到
			leoI2C_W_SDA(AckBit);	
			leoI2C_W_SCL(1);
			leoI2C_W_SCL(0);
}

uint8_t leoI2C_ReceiveAck(void)
{
			//接收应答：主机发送完一个字节后，在下一个时钟接收一位数据
			//					0表示从机收到，1表示未收到
			//					接受前先释放SDA，控制权给从机
			
			uint8_t AckBit;
			leoI2C_W_SDA(1);
			leoI2C_W_SCL(1);
			AckBit = leoI2C_R_SDA();
			leoI2C_W_SCL(0);
	
			return AckBit;
}
