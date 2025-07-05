#include "stm32f10x.h"
#include "OLED_Data.h"
#include "Delay.h"
#include <math.h>


/*此处的OLED屏显为简易版，只为显示数字等，图像与汉字按照图像存储
	没有用到缓存数组，因而不能自动实现变色、绘图等便于UI创作的功能
	能满足基本调试需求，切记，横128*像素，纵8*page*/


#define OLED_Address    0x78	//一般OLED屏的地址为0x78


//////////////////////////////////////////////////////////////////////////////////////////IIC驱动部分


#define oledI2C_SCL_GPIO GPIOB
#define oledI2C_SCL_PIN GPIO_Pin_8
#define oledI2C_SDA_GPIO GPIOB
#define oledI2C_SDA_PIN GPIO_Pin_9


//对读写进行函数封装
void oledI2C_W_SCL(uint8_t BitValue)	//写时钟线
{
			GPIO_WriteBit(oledI2C_SCL_GPIO, oledI2C_SCL_PIN,	(BitAction)BitValue);
			Delay_us(1);//方便高频移植
}

void oledI2C_W_SDA(uint8_t BitValue)	//写数据线
{
			GPIO_WriteBit(oledI2C_SDA_GPIO, oledI2C_SDA_PIN,	(BitAction)BitValue);
			Delay_us(1);
}

uint8_t	oledI2C_R_SDA(void)	//读数据线
{
			uint8_t BitValue;
			BitValue = GPIO_ReadInputDataBit(oledI2C_SDA_GPIO, oledI2C_SDA_PIN);
			Delay_us(1);
			return BitValue;
}

void oledI2C_Init(void)	//I2C初始化
{
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
			GPIO_InitTypeDef GPIO_InitStructure;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //把SCL和SDA初始化为开漏输出模式
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
			GPIO_InitStructure.GPIO_Pin = oledI2C_SCL_PIN;
			GPIO_Init(oledI2C_SCL_GPIO, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = oledI2C_SDA_PIN;
			GPIO_Init(oledI2C_SDA_GPIO, &GPIO_InitStructure);
	
			GPIO_SetBits(oledI2C_SCL_GPIO, oledI2C_SCL_PIN);
			GPIO_SetBits(oledI2C_SDA_GPIO, oledI2C_SDA_PIN);
}

//把I2C对应的6种时序基本单元对应
//SCL实质上是读写分离的表现，SCL为低时，往数据线里写，SCL为高时，从数据线里读
//当高电平时数据被修改，则特异性的为起始与终止

void oledI2C_Start(void)
{
			//起始条件：SCL与SDA高电平时，SDA从高电平切换到低电平,SCL再变为低电平
			oledI2C_W_SDA(1);//先释放SDA比较保险，为兼容重复起始条件，即SCL为低电平，SDA不确定，
											//当SDA也为低时，先释放SCL，再释放SDA为终止条件，导致混淆
	
			oledI2C_W_SCL(1);
			oledI2C_W_SDA(0);
			oledI2C_W_SCL(0);
}

void oledI2C_Stop(void)
{
			//终止条件：SCL与SDA低电平时，SCL从低电平切换到高电平,SDA再变为高电平
			oledI2C_W_SDA(0);//先拉低SDA比较保险，能确保产生上升沿
			oledI2C_W_SCL(1);
			oledI2C_W_SDA(1);
}

void oledI2C_SendByte(uint8_t Byte)
{
			//发送一个字节：SCL低电平期间，主机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，从机从SDA读取数据，主机再次拉低SCL，重复8次
			
			/*除结束标志外，SCL都以0结束，所以开始不用设SCL为0*/
			
			for(uint8_t i = 0; i < 8; i ++)
			{
				oledI2C_W_SDA(Byte & (0x80 >> i));
				oledI2C_W_SCL(1);
				oledI2C_W_SCL(0);	//不必担心跟不上，函数加了延时
			}
}

uint8_t oledI2C_ReceiveByte(void)
{
			//接收一个字节：SCL低电平期间，从机数据位高位先行，依次放在SDA线上，然后释放SCL
			//							SCL高位时保持SDA稳定，主机从SDA读取数据，主机再次拉低SCL，重复8次
			//							由于需要接收，主机每次读取前要释放SDA，将控制权交给从机
			
			uint8_t Byte = 0x00;
	
			oledI2C_W_SDA(1);
			for(uint8_t i = 0; i < 8; i ++)
			{
				
				oledI2C_W_SCL(1);
				if(oledI2C_R_SDA()) Byte |= (0x80 >> i);
				oledI2C_W_SCL(0);
			}
			
			return Byte;
}

void oledI2C_SendAck(uint8_t AckBit)
{
			//发送应答：主机接收完一个字节后，在下一个时钟发送一位数据
			//					0表示收到，1表示未收到
			oledI2C_W_SDA(AckBit);	
			oledI2C_W_SCL(1);
			oledI2C_W_SCL(0);
}

uint8_t oledI2C_ReceiveAck(void)
{
			//接收应答：主机发送完一个字节后，在下一个时钟接收一位数据
			//					0表示从机收到，1表示未收到
			//					接受前先释放SDA，控制权给从机
			
			uint8_t AckBit;
			oledI2C_W_SDA(1);
			oledI2C_W_SCL(1);
			AckBit = oledI2C_R_SDA();
			oledI2C_W_SCL(0);
	
			return AckBit;
}


//////////////////////////////////////////////////////////////////////////////////////////OLED驱动部分


void OLED_WrtCmd(uint8_t Cmd)
{
			//写命令：start -> 寻址 -> 接收应答 -> 非连续模式
			//				即 ctrl byte -> 接收应答 -> data or byte -> 接收应答 -> 终止
	
			oledI2C_Start();
			oledI2C_SendByte(OLED_Address);	//寻址
			oledI2C_ReceiveAck();
															//							 最高位为0 			次高位D/C为0，其余均为0
			oledI2C_SendByte(0x00);	//control byte，表示非连续模式，表示写命令，所以为0x00
			oledI2C_ReceiveAck();
	
			oledI2C_SendByte(Cmd);	//data byte，这里为方便，只写一个字节
			oledI2C_ReceiveAck();
	
			oledI2C_Stop();
}

void OLED_WrtData(uint8_t Data)
{
			//写数据：start -> 寻址 -> 接收应答 -> 非连续模式
			//				即 ctrl byte -> 接收应答 -> data or byte -> 接收应答 -> 终止
	
			oledI2C_Start();
			oledI2C_SendByte(OLED_Address);	//寻址
			oledI2C_ReceiveAck();
											
			oledI2C_SendByte(0x40);	//control byte，非连续模式写数据，次高位为1，所以为0x40
			oledI2C_ReceiveAck();
	
			oledI2C_SendByte(Data);	//data byte，这里为方便，只写一个字节
			oledI2C_ReceiveAck();
	
			oledI2C_Stop();
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
			uint32_t Result = 1;	//结果默认为1
			while (Y --)			//累乘Y次
			{
				Result *= X;		//每次把X累乘到结果上
			}
			return Result;
}

void OLED_Init(void)//初始化
{
			oledI2C_Init();
	
			Delay_ms(100);	//上电复位加延时
	
			OLED_WrtCmd(0xAE);	//显示关闭
	
			OLED_WrtCmd(0xD5);
			OLED_WrtCmd(0x80);	//双字节命令
	
			OLED_WrtCmd(0xA8);
			OLED_WrtCmd(0x3F);
	
			OLED_WrtCmd(0xD3);
			OLED_WrtCmd(0x00);
	
			OLED_WrtCmd(0x40);
			
			OLED_WrtCmd(0xA1);
	
			OLED_WrtCmd(0xC8);
			
			OLED_WrtCmd(0xDA);
			OLED_WrtCmd(0x12);
			
			OLED_WrtCmd(0x81);
			OLED_WrtCmd(0xCF);
			
			OLED_WrtCmd(0xD9);
			OLED_WrtCmd(0xF1);
			
			OLED_WrtCmd(0xDB);
			OLED_WrtCmd(0x30);
			
			OLED_WrtCmd(0xA4);
			
			OLED_WrtCmd(0xA6);
			
			OLED_WrtCmd(0x8D);
			OLED_WrtCmd(0x14);
			
			OLED_WrtCmd(0xAF);
			
			Delay_ms(100);
}

void OLED_SetCursor(uint8_t X, uint8_t Page)	//设置光标
{
			//命令高四位为0000时，低四位表示X坐标低位
			OLED_WrtCmd(0x00 | (X & 0x0F));
	
		
			//命令高四位为0001时，低四位表示X坐标高位
			OLED_WrtCmd(0x10 | ((X & 0xF0) >> 4));
	
			//命令高五位为1011 0时，低三位表示页地址值
			OLED_WrtCmd(0xB0 | Page);
}

void OLED_Clear(void)	//清屏
{
			for (uint8_t i = 0; i < 8; i ++)
			{
				OLED_SetCursor(0, i);
				for (uint8_t j = 0; j < 128; j ++)
				{
					OLED_WrtData(0x00);
				}
			}
}

void OLED_ShowChar(uint8_t X, uint8_t Page, char Char, uint8_t Size)	//显示字符
{
			if (Size == 6)
			{
				OLED_SetCursor(X, Page);
				
				for (uint8_t i = 0; i < 6; i ++)
				{
						OLED_WrtData(OLED_F6x8[Char - 0x20][i]);
				}
			}
			
			else if (Size == 8)
			{
				OLED_SetCursor(X, Page);
				
				for (uint8_t i = 0; i < 8; i ++)
				{
						OLED_WrtData(OLED_F8x16[Char - 0x20][i]);
				}
				
				OLED_SetCursor(X, Page + 1);
				
				for (uint8_t i = 8; i < 16; i ++)
				{
						OLED_WrtData(OLED_F8x16[Char - 0x20][i]);
				}
			}
}

void OLED_ShowString(uint8_t X, uint8_t Page, char* String, uint8_t Size)	//显示字符串
{
				for (uint8_t i = 0; String[i] != '\0'; i ++)
				{
					OLED_ShowChar(X + i * Size, Page, String[i], Size);
				}	
}

void OLED_ShowNum(uint8_t X, uint8_t Page, uint32_t Num, uint8_t Length, uint8_t Size)	//显示整数
{
				for (uint8_t i = 0; i < Length; i ++)		//遍历数字的每一位							
				{
					/*调用OLED_ShowChar函数，依次显示每个数字*/
					/*Number / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
					/*+ '0' 可将数字转换为字符格式*/
					OLED_ShowChar(X + i * Size, Page, Num / OLED_Pow(10, Length - i - 1) % 10 + '0', Size);
				}
}

void OLED_ShowSignedNum(uint8_t X, uint8_t Page, int32_t Num, uint8_t Length, uint8_t Size)	//显示带符号整数
{
				uint32_t Num1;
				
				if (Num >= 0)						//数字大于等于0
				{
					OLED_ShowChar(X, Page, '+', Size);	//显示+号
					Num1 = Num;					//Number1直接等于Number
				}
				
				else									//数字小于0
				{
					OLED_ShowChar(X, Page, '-', Size);	//显示-号
					Num1 = -Num;					//Number1等于Number取负
				}
				
				for (uint8_t i = 0; i < Length; i++)			//遍历数字的每一位								
				{
					/*调用OLED_ShowChar函数，依次显示每个数字*/
					/*Number1 / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
					/*+ '0' 可将数字转换为字符格式*/
					OLED_ShowChar(X + (i + 1) * Size, Page, Num1 / OLED_Pow(10, Length - i - 1) % 10 + '0', Size);
				}
}

void OLED_ShowFloatNum(uint8_t X, uint8_t Page, double Num, uint8_t IntLength, uint8_t FraLength, uint8_t Size)	//显示带符号小数
{
				uint32_t PowNum, IntNum, FraNum;
				
				if (Num >= 0)						//数字大于等于0
				{
					OLED_ShowChar(X, Page, '+', Size);	//显示+号
				}
				
				else									//数字小于0
				{
					OLED_ShowChar(X, Page, '-', Size);	//显示-号
					Num = -Num;					//Number取负
				}
				
				/*提取整数部分和小数部分*/
				IntNum = Num;						//直接赋值给整型变量，提取整数
				Num -= IntNum;						//将Number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
				PowNum = OLED_Pow(10, FraLength);		//根据指定小数的位数，确定乘数
				FraNum = round(Num * PowNum);		//将小数乘到整数，同时四舍五入，避免显示误差
				IntNum += FraNum / PowNum;				//若四舍五入造成了进位，则需要再加给整数
				
				/*显示整数部分*/
				OLED_ShowNum(X + Size, Page, IntNum, IntLength, Size);
				
				/*显示小数点*/
				OLED_ShowChar(X + (IntLength + 1) * Size, Page, '.', Size);
				
				/*显示小数部分*/
				OLED_ShowNum(X + (IntLength + 2) * Size, Page, FraNum, FraLength, Size);
}

void OLED_ShowHexNum(uint8_t X, uint8_t Page, uint32_t Num, uint8_t Length, uint8_t Size)	//显示十六进制数
{
				uint8_t  SingleNum;
				for (uint8_t i = 0; i < Length; i ++)		//遍历数字的每一位
				{
					/*以十六进制提取数字的每一位*/
					SingleNum = Num / OLED_Pow(16, Length - i - 1) % 16;
					
					if (SingleNum < 10)			//单个数字小于10
					{
						/*调用OLED_ShowChar函数，显示此数字*/
						/*+ '0' 可将数字转换为字符格式*/
						OLED_ShowChar(X + i * Size, Page, SingleNum + '0', Size);
					}
					else							//单个数字大于10
					{
						/*调用OLED_ShowChar函数，显示此数字*/
						/*+ 'A' 可将数字转换为从A开始的十六进制字符*/
						OLED_ShowChar(X + i * Size, Page, SingleNum - 10 + 'A', Size);
					}
				}
}

void OLED_ShowImage(uint8_t X, uint8_t Page, uint8_t Width, uint8_t HeightByPage, const uint8_t* Image)	//显示图像
{
				for (uint8_t i = 0; i < HeightByPage; i ++)
				{
					OLED_SetCursor(X, Page + i);
				
					for (uint8_t j = 0; j < Width; j ++)
					{
						OLED_WrtData(Image[Width * i + j]);
					}
				}
}

void OLED_OpenShow(void)//开机动画
{
			OLED_Clear();
			OLED_ShowImage(0, 0, 64, 8, NWPU);
			Delay_ms(200);
			OLED_ShowImage(64, 0, 64, 4, xc);
			Delay_ms(200);
			OLED_ShowImage(64, 4, 64, 4, tk);
			Delay_ms(1600);
			OLED_Clear();
}
