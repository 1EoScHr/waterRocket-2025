#include "stm32f10x.h"
#include "MPU6050.h"
#include "MPU6050_Reg.h"
#include "Delay.h"
#include <math.h>

#define mpuI2C_SCL_GPIO GPIOB
#define mpuI2C_SCL_PIN GPIO_Pin_11
#define mpuI2C_SDA_GPIO GPIOB
#define mpuI2C_SDA_PIN GPIO_Pin_10

void mpuI2C_W_SCL(uint8_t BitValue)	//写时钟线
{
			GPIO_WriteBit(mpuI2C_SCL_GPIO, mpuI2C_SCL_PIN,	(BitAction)BitValue);
			Delay_us(1);//方便高频移植
}

void mpuI2C_W_SDA(uint8_t BitValue)	//写数据线
{
			GPIO_WriteBit(mpuI2C_SDA_GPIO, mpuI2C_SDA_PIN,	(BitAction)BitValue);
			Delay_us(1);
}

uint8_t	mpuI2C_R_SDA(void)	//读数据线
{
			uint8_t BitValue;
			BitValue = GPIO_ReadInputDataBit(mpuI2C_SDA_GPIO, mpuI2C_SDA_PIN);
			Delay_us(1);
			return BitValue;
}

void mpuI2C_Init(void)	//I2C初始化
{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //把SCL和SDA初始化为开漏输出模式
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
			GPIO_InitStructure.GPIO_Pin = mpuI2C_SCL_PIN;
			GPIO_Init(mpuI2C_SCL_GPIO, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = mpuI2C_SDA_PIN;
			GPIO_Init(mpuI2C_SDA_GPIO, &GPIO_InitStructure);
	
			GPIO_SetBits(mpuI2C_SCL_GPIO, mpuI2C_SCL_PIN);
			GPIO_SetBits(mpuI2C_SDA_GPIO, mpuI2C_SDA_PIN);
}

//把I2C对应的6种时序基本单元对应
//SCL实质上是读写分离的表现，SCL为低时，往数据线里写，SCL为高时，从数据线里读
//当高电平时数据被修改，则特异性的为起始与终止

void mpuI2C_Start(void)
{
			//起始条件：SCL与SDA高电平时，SDA从高电平切换到低电平,SCL再变为低电平
			mpuI2C_W_SDA(1);//先释放SDA比较保险，为兼容重复起始条件，即SCL为低电平，SDA不确定，
											//当SDA也为低时，先释放SCL，再释放SDA为终止条件，导致混淆
	
			mpuI2C_W_SCL(1);
			mpuI2C_W_SDA(0);
			mpuI2C_W_SCL(0);
}

void mpuI2C_Stop(void)
{
			//终止条件：SCL与SDA低电平时，SCL从低电平切换到高电平,SDA再变为高电平
			mpuI2C_W_SDA(0);//先拉低SDA比较保险，能确保产生上升沿
			mpuI2C_W_SCL(1);
			mpuI2C_W_SDA(1);
}

void mpuI2C_SendByte(uint8_t Byte)
{
			//发送一个字节：SCL低电平期间，主机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，从机从SDA读取数据，主机再次拉低SCL，重复8次
			
			/*除结束标志外，SCL都以0结束，所以开始不用设SCL为0*/
			
			for(uint8_t i = 0; i < 8; i ++)
			{
				mpuI2C_W_SDA(Byte & (0x80 >> i));
				mpuI2C_W_SCL(1);
				mpuI2C_W_SCL(0);	//不必担心跟不上，函数加了延时
			}
}

uint8_t mpuI2C_ReceiveByte(void)
{
			//接收一个字节：SCL低电平期间，从机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，主机从SDA读取数据，主机再次拉低SCL，重复8次
			//							由于需要接收，主机每次读取前要释放SDA，将控制权交给从机
			
			uint8_t Byte = 0x00;
	
			mpuI2C_W_SDA(1);
			for(uint8_t i = 0; i < 8; i ++)
			{
				
				mpuI2C_W_SCL(1);
				if(mpuI2C_R_SDA()) Byte |= (0x80 >> i);
				mpuI2C_W_SCL(0);
			}
			
			return Byte;
}

void mpuI2C_SendAck(uint8_t AckBit)
{
			//发送应答：主机接收完一个字节后，在下一个时钟发送一位数据
			//					0表示收到，1表示未收到
			mpuI2C_W_SDA(AckBit);	
			mpuI2C_W_SCL(1);
			mpuI2C_W_SCL(0);
}

uint8_t mpuI2C_ReceiveAck(void)
{
			//接收应答：主机发送完一个字节后，在下一个时钟接收一位数据
			//					0表示从机收到，1表示未收到
			//					接受前先释放SDA，控制权给从机
			
			uint8_t AckBit;
			mpuI2C_W_SDA(1);
			mpuI2C_W_SCL(1);
			AckBit = mpuI2C_R_SDA();
			mpuI2C_W_SCL(0);
	
			return AckBit;
}

void MPU6050_WriteByte2Reg(uint8_t RegAddress, uint8_t Byte)
{
			//指定从机地址写指定寄存器，拼接时序：start -> 送MPU6050地址 -> 接收应答 -> 
			//																		送指定寄存器地址 -> 接收应答 -> 送指定数据
			//																		-> 接收应答 -> stop
			
			mpuI2C_Start();
			mpuI2C_SendByte(MPU6050_Address);
			mpuI2C_ReceiveAck();
			mpuI2C_SendByte(RegAddress);
			mpuI2C_ReceiveAck();	//指定地址
	
			mpuI2C_SendByte(Byte);
			mpuI2C_ReceiveAck();
			mpuI2C_Stop();
}

uint8_t MPU6050_ReadByte2Reg(uint8_t RegAddress)
{
			//指定从机地址读指定寄存器，拼接时序：start -> 送MPU6050地址 -> 接收应答 -> 
			//																		送指定寄存器地址 -> 接收应答 -> restart 
			//																		-> 送指定读写位 -> 接收应答 -> 接收数据
			//																		-> 发送应答 -> stop

			uint8_t Byte;
			
			mpuI2C_Start();
			mpuI2C_SendByte(MPU6050_Address);
			mpuI2C_ReceiveAck();
			mpuI2C_SendByte(RegAddress);
			mpuI2C_ReceiveAck();	//指定地址
			
			mpuI2C_Start();
			mpuI2C_SendByte(MPU6050_Address | 0x01);	//指定从机地址与读写位，变为读
			mpuI2C_ReceiveAck();
			Byte = mpuI2C_ReceiveByte();
			mpuI2C_SendAck(1);	//因为这里只读一个字节，所以不给应答；若需要读取多个，则给应答
			mpuI2C_Stop();
			
			return Byte;
}

//void MPU6050_WriteData2Reg(uint8_t RegAddress, uint8_t Data)
//{
//			//指定从机地址写指定寄存器，多位
//}

//uint8_t MPU6050_ReadData2Reg(uint8_t RegAddress)
//{
//			//指定从机地址读指定寄存器，多位
//			return 0x00;
//}

void MPU6050_Init(void)
{
			mpuI2C_Init();
			
/*根据实际项目需求更改*/
	
			//电源管理寄存器1
			//0				0				0				0			 	0(不失能)	000/001
			//不复位	不睡眠	不循环	无关位	温度传		选时钟
			MPU6050_WriteByte2Reg(MPU6050_PWR_MGMT_1, 0x01);	
	
			//电源管理寄存器2
			//00(不需要)		000000(六轴都不)
			//循环唤醒频率	每个轴待机位
			MPU6050_WriteByte2Reg(MPU6050_PWR_MGMT_2, 0x00);
	
			//采样率分频寄存器1
			//0x09，十分频（分频系数=1+该寄存器值）
			//大小决定数据输出快慢，越小越快
			MPU6050_WriteByte2Reg(MPU6050_SMPLRT_DIV, 0x09);
	
			//配置寄存器
			//00		 000(不需要)	110(最平滑)
			//无关位	外部同步		数字低通滤波
			MPU6050_WriteByte2Reg(MPU6050_CONFIG, 0x06);
			
			//陀螺仪配置寄存器
			//000(不了)	11(最大)		000
			//自测使能	满量程选择	无关位		(量程选择：250，500，1000，2000 °/s)
			MPU6050_WriteByte2Reg(MPU6050_GYRO_CONFIG, 0x18);
			
			//加速度计配置寄存器
			//000(不了)	11(最大)		00(不用)
			//自测使能	满量程选择	高通滤波	(量程选择：2，4，8，16 g)
			MPU6050_WriteByte2Reg(MPU6050_ACCEL_CONFIG, 0x18);
}

uint8_t MPU6050_ReadID(void)
{
			return MPU6050_ReadByte2Reg(MPU6050_WHO_AM_I);
}


void MPU6050_GetData(MPU6050_Data* Data)	//获取6个数据，分别表示XYZ加速度与陀螺仪值
{
			//读取X轴加速度寄存器高八位与低八位
			Data->AX = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_ACCEL_XOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_ACCEL_XOUT_L);
	
			//读取Y轴
			Data->AY = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_ACCEL_YOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_ACCEL_YOUT_L);
	
			//读取Z轴
			Data->AZ = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_ACCEL_ZOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_ACCEL_ZOUT_L);
	
			//读取X轴陀螺仪寄存器高八位与低八位
			Data->GX = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_GYRO_XOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_GYRO_XOUT_L);
	
			//读取Y轴
			Data->GY = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_GYRO_YOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_GYRO_YOUT_L);
	
			//读取Z轴
			Data->GZ = ((int16_t)MPU6050_ReadByte2Reg(MPU6050_GYRO_ZOUT_H) << 8) + MPU6050_ReadByte2Reg(MPU6050_GYRO_ZOUT_L);
}

float MPU6050_CaculateAccel(MPU6050_Data *Data)	//
{
			//MPU6050获取到的数据换算：
	
			//读取到的数据  _	 		 X
			// ----------   _  ---------
			//   32768					 满量程
			
			//初始化时，定义的均为最大量程
	
			return sqrt((Data->AX)*(Data->AX)+(Data->AY)*(Data->AY)+(Data->AZ)*(Data->AZ)) * 0.00048828125;
}

