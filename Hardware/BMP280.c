#include "stm32f10x.h"                  // Device header
#include "BMP280.h"
#include "BMP280_Reg.h"
#include "Delay.h"
#include <math.h>

//用来在初始化函数中记录芯片内矫正参数
BMP280 bmp;
BMP280* bmp280 = &bmp;	//先定义再指向


//////////////////////////////////////////////////////////////////////////////////////////IIC驱动部分


#define bmpI2C_SCL_GPIO GPIOB
#define bmpI2C_SCL_PIN GPIO_Pin_13
#define bmpI2C_SDA_GPIO GPIOB
#define bmpI2C_SDA_PIN GPIO_Pin_12


//对读写进行函数封装
void bmpI2C_W_SCL(uint8_t BitValue)	//写时钟线
{
			GPIO_WriteBit(bmpI2C_SCL_GPIO, bmpI2C_SCL_PIN,	(BitAction)BitValue);
			Delay_us(1);//方便高频移植
}

void bmpI2C_W_SDA(uint8_t BitValue)	//写数据线
{
			GPIO_WriteBit(bmpI2C_SDA_GPIO, bmpI2C_SDA_PIN,	(BitAction)BitValue);
			Delay_us(1);
}

uint8_t	bmpI2C_R_SDA(void)	//读数据线
{
			uint8_t BitValue;
			BitValue = GPIO_ReadInputDataBit(bmpI2C_SDA_GPIO, bmpI2C_SDA_PIN);
			Delay_us(1);
			return BitValue;
}

void bmpI2C_Init(void)	//I2C初始化
{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //把SCL和SDA初始化为开漏输出模式
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
			GPIO_InitStructure.GPIO_Pin = bmpI2C_SCL_PIN;
			GPIO_Init(bmpI2C_SCL_GPIO, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = bmpI2C_SDA_PIN;
			GPIO_Init(bmpI2C_SDA_GPIO, &GPIO_InitStructure);
	
			GPIO_SetBits(bmpI2C_SCL_GPIO, bmpI2C_SCL_PIN);
			GPIO_SetBits(bmpI2C_SDA_GPIO, bmpI2C_SDA_PIN);
}


//把I2C对应的6种时序基本单元对应
//SCL实质上是读写分离的表现，SCL为低时，往数据线里写，SCL为高时，从数据线里读
//当高电平时数据被修改，则特异性的为起始与终止

void bmpI2C_Start(void)
{
			//起始条件：SCL与SDA高电平时，SDA从高电平切换到低电平,SCL再变为低电平
			bmpI2C_W_SDA(1);//先释放SDA比较保险，为兼容重复起始条件，即SCL为低电平，SDA不确定，
											//当SDA也为低时，先释放SCL，再释放SDA为终止条件，导致混淆
	
			bmpI2C_W_SCL(1);
			bmpI2C_W_SDA(0);
			bmpI2C_W_SCL(0);
}

void bmpI2C_Stop(void)
{
			//终止条件：SCL与SDA低电平时，SCL从低电平切换到高电平,SDA再变为高电平
			bmpI2C_W_SDA(0);//先拉低SDA比较保险，能确保产生上升沿
			bmpI2C_W_SCL(1);
			bmpI2C_W_SDA(1);
}

void bmpI2C_SendByte(uint8_t Byte)
{
			//发送一个字节：SCL低电平期间，主机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，从机从SDA读取数据，主机再次拉低SCL，重复8次
			
			/*除结束标志外，SCL都以0结束，所以开始不用设SCL为0*/
			
			for(uint8_t i = 0; i < 8; i ++)
			{
				bmpI2C_W_SDA(Byte & (0x80 >> i));
				bmpI2C_W_SCL(1);
				bmpI2C_W_SCL(0);	//不必担心跟不上，函数加了延时
			}
}

uint8_t bmpI2C_ReceiveByte(void)
{
			//接收一个字节：SCL低电平期间，从机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，主机从SDA读取数据，主机再次拉低SCL，重复8次
			//							由于需要接收，主机每次读取前要释放SDA，将控制权交给从机
			
			uint8_t Byte = 0x00;
	
			bmpI2C_W_SDA(1);
			for(uint8_t i = 0; i < 8; i ++)
			{
				
				bmpI2C_W_SCL(1);
				if(bmpI2C_R_SDA()) Byte |= (0x80 >> i);
				bmpI2C_W_SCL(0);
			}
			
			return Byte;
}

void bmpI2C_SendAck(uint8_t AckBit)
{
			//发送应答：主机接收完一个字节后，在下一个时钟发送一位数据
			//					0表示收到，1表示未收到
			bmpI2C_W_SDA(AckBit);	
			bmpI2C_W_SCL(1);
			bmpI2C_W_SCL(0);
}

uint8_t bmpI2C_ReceiveAck(void)
{
			//接收应答：主机发送完一个字节后，在下一个时钟接收一位数据
			//					0表示从机收到，1表示未收到
			//					接受前先释放SDA，控制权给从机
			
			uint8_t AckBit;
			bmpI2C_W_SDA(1);
			bmpI2C_W_SCL(1);
			AckBit = bmpI2C_R_SDA();
			bmpI2C_W_SCL(0);
	
			return AckBit;
}

void BMP280_WriteByte2Reg(uint8_t RegAddress, uint8_t Byte)
{
			//指定从机地址写指定寄存器，拼接时序：start -> 送BMP280地址 -> 接收应答 -> 
			//																		送指定寄存器地址 -> 接收应答 -> 送指定数据
			//																		-> 接收应答 -> stop
			
			bmpI2C_Start();
			bmpI2C_SendByte(BMP280_ADDRESS);
			bmpI2C_ReceiveAck();
			bmpI2C_SendByte(RegAddress);
			bmpI2C_ReceiveAck();	//指定地址
	
			bmpI2C_SendByte(Byte);
			bmpI2C_ReceiveAck();
			bmpI2C_Stop();
}

uint8_t BMP280_ReadByte2Reg(uint8_t RegAddress)
{
			//指定从机地址读指定寄存器，拼接时序：start -> 送BMP280地址 -> 接收应答 -> 
			//																		送指定寄存器地址 -> 接收应答 -> restart 
			//																		-> 送指定读写位 -> 接收应答 -> 接收数据
			//																		-> 发送应答 -> stop

			uint8_t Byte;
			
			bmpI2C_Start();
			bmpI2C_SendByte(BMP280_ADDRESS);
			bmpI2C_ReceiveAck();
			bmpI2C_SendByte(RegAddress);
			bmpI2C_ReceiveAck();	//指定地址
			
			bmpI2C_Start();
			bmpI2C_SendByte(BMP280_ADDRESS | 0x01);	//指定从机地址与读写位，变为读
			bmpI2C_ReceiveAck();
			Byte = bmpI2C_ReceiveByte();
			bmpI2C_SendAck(1);	//因为这里只读一个字节，所以不给应答；若需要读取多个，则给应答
			bmpI2C_Stop();
			
			return Byte;
}


//////////////////////////////////////////////////////////////////////////////////////////bmp280驱动部分


void BMP280_Init(void)	//BMP280初始化
{
			bmpI2C_Init();
	
			//读温度传感器矫正值
			bmp280->T1 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_T1_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_T1_LSB_REG);			//高位加低位
			bmp280->T2 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_T2_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_T2_LSB_REG);
			bmp280->T3 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_T3_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_T3_LSB_REG);
																																					
			//读气压传感器矫正值                                                
			bmp280->P1 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P1_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P1_LSB_REG);
			bmp280->P2 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P2_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P2_LSB_REG);
			bmp280->P3 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P3_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P3_LSB_REG);
			bmp280->P4 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P4_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P4_LSB_REG);
			bmp280->P5 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P5_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P5_LSB_REG);
			bmp280->P6 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P6_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P6_LSB_REG);
			bmp280->P7 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P7_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P7_LSB_REG);
			bmp280->P8 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P8_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P8_LSB_REG);
			bmp280->P9 = (((uint16_t)BMP280_ReadByte2Reg(BMP280_DIG_P9_MSB_REG))<<8) | BMP280_ReadByte2Reg(BMP280_DIG_P9_LSB_REG);
			
			//复位
			BMP280_WriteByte2Reg(BMP280_RESET_REG, BMP280_RESET_VALUE);
	
			//配置测量控制寄存器
			//	001	(x1)	 		011(x4)			11(normal)
			//温度过采样  	压力过采样  	工作模式
			BMP280_WriteByte2Reg(BMP280_CTRLMEAS_REG, 0x2F);
			
			
			//配置config寄存器
			//000(0.5ms)			100(x16)					0				0(不设置)
			//保持时间SB	 	滤波器分频因子		无关位		SPI设置
			BMP280_WriteByte2Reg(BMP280_CONFIG_REG, 0x10);
}

void BMP280_WaitRead(void)	//等待转换完成以读取数据
{
			while (BMP280_ReadByte2Reg(BMP280_STATUS_REG) == BMP280_MEASURING 
					|| BMP280_ReadByte2Reg(BMP280_STATUS_REG) == BMP280_IM_UPDATE)	//如果正在测量或更新，则等到可以读取
			{
				
			}
}

uint8_t BMP280_ReadID(void)
{
	return BMP280_ReadByte2Reg(BMP280_CHIPID_REG);
}

void BMP280_GetData(double *Temp, double *Pres)	//获取寄存器内的原始数据并修正转换为准确值
{
			int32_t adc_T, adc_P, BMP280_t_fine;	//寄存器内读出的原始值，温度与气压的传递变量
			double var1, var2, p;
	
			BMP280_WaitRead();
			adc_T = (uint32_t) (BMP280_ReadByte2Reg(BMP280_TEMPERATURE_MSB_REG) << 12) 
								| (uint32_t) (BMP280_ReadByte2Reg(BMP280_TEMPERATURE_LSB_REG) << 4)
								| (uint32_t) (BMP280_ReadByte2Reg(BMP280_TEMPERATURE_XLSB_REG ) >> 4);
			
			var1 = (((double)adc_T) / 16384.0 - ((double)bmp280->T1) / 1024.0) * ((double)bmp280->T2);
			var2 = ((((double)adc_T) / 131072.0 - ((double)bmp280->T1) /8192.0) * (((double)adc_T)
                / 131072.0 - ((double)bmp280->T1) / 8192.0)) * ((double)bmp280->T3);
			
			BMP280_t_fine = (int32_t)(var1 + var2);
			*Temp = (var1 + var2) / 5120.0;	//得到温度
	
			BMP280_WaitRead();
			adc_P = (int32_t) (BMP280_ReadByte2Reg(BMP280_PRESSURE_MSB_REG) << 12) 
								+ (int32_t) (BMP280_ReadByte2Reg(BMP280_PRESSURE_LSB_REG) << 4)
								+ (int32_t) (BMP280_ReadByte2Reg(BMP280_PRESSURE_XLSB_REG ) >> 4);

			var1 = ((double)BMP280_t_fine / 2.0) - 64000.0;
			var2 = var1 * var1 * ((double)bmp280->P6) / 32768.0;
			var2 = var2 + var1 * ((double)bmp280->P5) * 2.0;
			var2 = (var2 / 4.0) + (((double)bmp280->P4) * 65536.0);
			var1 = (((double)bmp280->P3) * var1 * var1 / 524288.0 + ((double)bmp280->P2) * var1) / 524288.0;
			var1 = (1.0 + var1 /32768.0) * ((double)bmp280->P1);
			if (var1 == 0.0)
			{
				
			}
			p = 1048576.0 - (double)adc_P;
			p = (p - (var2 / 4096.0)) * 6250.0 /	var1;
			var1 = ((double)bmp280->P9) * p * p / 2147483648.0;
			var2 = p * ((double)bmp280->P8) / 32768.0;
			*Pres = p + (var1 + var2 + ((double)bmp280->P7)) / 16.0;
}

uint16_t BMP280_GetHeight(void)
{
			double Temp, Pres;
			uint16_t Height;
			BMP280_GetData(&Temp, &Pres);
			Height = 153.8* (( pow( 101325.0f/ Pres, 1.0f / 5.255f)-1)*(273.2+ Temp));
			return Height;
}
