#include <stdint.h>
#include <string.h>
#include <math.h>

#include "stm32f10x.h"
#include "LED.h"
#include "Key.h"
#include "Beep.h"
#include "Timer.h"
#include "Delay.h"
#include "Servo.h"
#include "Serial.h"
#include "BMP280.h"
#include "W25Q64.h"
#include "MPU6050.h"
#include "ShortLine.h"
#include "OLED_Simple.h"

MPU6050_Data Data;

#define DATA_HEAD_FLAG 0x0F0F

uint32_t d_second = 0;
uint8_t dummy[4];
uint8_t accel[256];
uint8_t height[256];
uint8_t workModeFlag = 0;
uint8_t htInd = 2, acInd = 2;

void ErrorHandle(void)	// 报错入口
{
	LED13_Init();
	Beep_Init();

	LED13_on();Beep_JustLoud();

	while(1)
	{}
}

uint16_t flash2flag(uint32_t addr)	// 将flash指定位置中的2个u8解读为u16，只用于找包头
{
		W25Q64_ReadData(addr, dummy, 2);
		return (dummy[0] << 8 | dummy[1]);
}

void ht2arr(uint16_t u16, uint8_t* u8_arr, uint8_t ind)	// 将u16写进指定u8数组的ind索引处
{
		u8_arr[2*ind] = u16 >> 8;
		u8_arr[2*ind+1] = u16;
}

float arr2ac(uint8_t* arr, uint8_t ind)// 将arr中指定ind处的4个u8解读为float
{
    float result;
    memcpy(&result, arr+4*ind, sizeof(float));
    return result;
}

void ac2arr(float acc, uint8_t* u8_arr, uint8_t ind)// 将float写进指定u8数组的ind索引处
{
    memcpy(&u8_arr[4 * ind], &acc, sizeof(float));
}


void Self_Detect(void) 	// 硬件自检
{
		uint8_t i = 3;
		
		LED13_Init();				// 基础硬件：user led，beep
		Beep_Init();
		while(i)
		{
				LED13_on();Beep_JustLoud();
				Delay_ms(250);
				LED13_off();Beep_Stop();
				Delay_ms(250);
				i --;
		}
	
		Beep_JustLoud();LED13_on();Delay_ms(500);Beep_Stop();LED13_off();
		
		OLED_Init();				// 交互硬件：OLED
		OLED_OpenShow();
		
		Beep_JustLoud();LED13_on();Delay_ms(500);Beep_Stop();LED13_off();
		
		ShortLine_Init();		// 一般硬件：shortline
		OLED_ShowString(0, 0, "cnct stLine plz", SML);
		while(!ShortLine())
		{}
		OLED_ShowString(0, 0, "plug", SML);
		while(ShortLine())
		{}
		OLED_Clear();
		
		Beep_JustLoud();LED13_on();Delay_s(1);Beep_Stop();LED13_off();
		
		BMP280_Init();			// 重要硬件：气压计
		OLED_ShowString(0, 0, "BMP280: ", BIG);
		if(BMP280_ReadID() != 0x58)
		{
				OLED_ShowString(81, 0, "Wrong!", BIG);
				LED13_on();Beep_JustLoud();
				while(1)
				{}
		}
		
		else
		{
				OLED_ShowString(65, 0, "Yes", BIG);
		}	
		
		uint16_t Height = BMP280_GetHeight();
		OLED_ShowString(0, 3, "ht of now:", SML);
		OLED_ShowNum(65, 2, Height, 3, BIG);
		OLED_ShowString(0, 5, "blow me, plz", SML);
		
		uint16_t newHeight = BMP280_GetHeight();
		while(newHeight - Height < 5)
		{
				OLED_ShowNum(65, 2, BMP280_GetHeight(), 3, BIG);
				newHeight = BMP280_GetHeight();
		}

		OLED_ShowString(0, 6, "OK OK OK", BIG);
		Delay_ms(1000);
		
		OLED_Clear();
		MPU6050_Init();
		OLED_ShowString(0, 0, "MPU6050: ", BIG);
		
		if(MPU6050_ReadID() != 0x68)
		{
				OLED_ShowString(72, 0, "Wrong!", BIG);
				LED13_on();Beep_JustLoud();
				while(1)
				{}
		}
		else
		{
				OLED_ShowString(65, 0, "Yes", BIG);
		}	
		
		MPU6050_GetData(&Data);
		float acc = MPU6050_CaculateAccel(&Data);
		OLED_ShowString(0, 3, "acc of now:", SML);
		OLED_ShowFloatNum(71, 2, acc, 2, 2, BIG);
		OLED_ShowString(0, 5, "shake me, plz", SML);
		
		MPU6050_GetData(&Data);
		float newAcc = MPU6050_CaculateAccel(&Data);
		while(newAcc - acc < 3)
		{
				OLED_ShowFloatNum(71, 2, newAcc, 2, 2, BIG);
				MPU6050_GetData(&Data);
				newAcc = MPU6050_CaculateAccel(&Data);
		}

		OLED_ShowString(0, 6, "OK OK OK", BIG);
		Delay_ms(1000);
		
		
		OLED_Clear();
		W25Q64_Init();
		OLED_ShowString(0, 0, "W25Q64: ", BIG);
		uint8_t MID;
		uint16_t DID;
		W25Q64_ReadID(&MID, &DID);
		OLED_ShowString(0, 3, "MID:", SML);
		OLED_ShowHexNum(33, 3, MID, 2, SML);
		OLED_ShowString(0, 4, "DID:", SML);
		OLED_ShowHexNum(33, 4, DID, 2, SML);
}

void EscapeTower(void)	// 逃逸塔
{
		uint8_t lastInd = 0;
	
		while(d_second < 11)
		{
				if(lastInd == acInd)
				{
						continue;
				}
				
				lastInd = acInd;
				
				if(arr2ac(accel, acInd) > 5.0)
				{
						return;
				}
		}
		
		Dlg_relese();
		Para_relese();
		OLED_ShowString(0, 2, "Oh, escapeTower trig :(  ", SML);
		ErrorHandle();
}

/*

h = a*t*t + b*t + c



*/










void Height_Flash_Control(void)	// 飞控
{
		//----------------------------------------------初始化

//		Servo1_SetAngle(0);
//		Servo2_SetAngle(0);

		uint32_t addr_start = 0x000000;

		//----------------------------------------------确定起始地址

		while(flash2flag(addr_start) == DATA_HEAD_FLAG)
		{
			addr_start += 0x000100;
		}
		
		OLED_ShowString(0, 0, "addr:", SML);
		OLED_ShowHexNum(31, 0, addr_start, 6, SML);

		//----------------------------------------------数据准备

		ht2arr(0x0F0F, height, 0);
		ht2arr(0x0F0F, accel, 0);
		uint16_t height_begin;
		float acc;

		//----------------------------------------------上架静默时间，防止逃逸塔误触

//		for(uint8_t i = 0; i < 15; i ++)
//		{
//			LED13_on();Beep_JustLoud();
//			Delay_ms(900);
//			LED13_off();Beep_Stop();
//			Delay_ms(900);
//		}

		//----------------------------------------------接线提示&初始高度
		
		OLED_ShowString(0, 2, "take off, plz", SML);
		
		acc = MPU6050_CaculateAccel(&Data);
		
		LED13_on();Beep_JustLoud();
		while (acc < 2.1)
		{
			height_begin = BMP280_GetHeight();
			OLED_ShowNum(0, 4, height_begin, 3, BIG);
			acc = MPU6050_CaculateAccel(&Data);
			OLED_ShowFloatNum(30, 4, acc, 2, 2, BIG);
		}
		LED13_off();Beep_Stop();
		
		OLED_ShowString(0, 2, "OK, thanks :)    ", SML);

		ht2arr(height_begin, height, 1);
		ac2arr(acc, accel, 1);
		

		//----------------------------------------------起飞后

		
		Timer_Internal_Init();
		
		//EscapeTower(); // 第一秒在逃逸塔内

		while(d_second < 121)
		{
			OLED_ShowNum(64, 7, d_second, 3, SML);
		}		

		Timer_Internal_DeInit();
		
		W25Q64_PageProgram(addr_start, height, 256);
		W25Q64_PageProgram(addr_start+0x000100, accel, 256);
		OLED_ShowString(0, 6, "Done.", BIG);
}

void Flash_ReadLast(void)	// 显示上次飞行高度数据
{
		OLED_Clear();
		
		uint32_t addr_emp = 0x000000;
		if(flash2flag(addr_emp) != 0x0F0F)
		{
			OLED_ShowString(0, 0, "Warning:", SML);
			OLED_ShowString(0, 2, "No data in flash!", SML);
			return;
		}

		uint32_t addr_use = addr_emp;
		addr_emp += 0x000100;

		while (flash2flag(addr_emp) == 0x0F0F)
		{
			addr_use = addr_emp;
			addr_emp += 0x000100;
		}
		
		OLED_ShowString(0, 0, "read ht addr: ", SML);
		OLED_ShowHexNum(85,0, addr_use-0x000100, 6, SML);
		OLED_ShowString(0, 1, "read ac addr: ", SML);
		OLED_ShowHexNum(85,1, addr_use, 6, SML);
		
		W25Q64_ReadData(addr_use-0x000100, height, 256);
		uint16_t hst = height[2]<<8 | height[3];
		OLED_ShowString(0, 3, "start:", SML);
		OLED_ShowNum(37, 3, hst, 3, BIG);

		for(uint8_t ind = 2; ind < 128; ind ++)
		{
			if(hst <  (height[2*ind]<<8 | height[2*ind+1]))
			{
				hst = (height[2*ind]<<8 | height[2*ind+1]);
			}
		}

		OLED_ShowString(71, 3, "hmo:", SML);
		OLED_ShowNum(95, 3, hst, 3, BIG);		
		
		W25Q64_ReadData(addr_use, accel, 256);
		float acst = arr2ac(accel, 1);
		OLED_ShowString(0, 6, "start:", SML);
		OLED_ShowFloatNum(37, 6, acst, 1, 2, SML);

		for(uint8_t ind = 2; ind < 64; ind ++)
		{
			if(acst <  arr2ac(accel, ind))
			{
				acst = arr2ac(accel, ind);
			}
		}

		OLED_ShowString(72, 6, "amo:", SML);
		OLED_ShowFloatNum(96, 6, acst, 2, 1, SML);			
}


void Data_UART2PC(void)
{
		OLED_Clear();
	
		uint32_t addr = 0x000000;
		uint8_t ind = 1;
	
		while (flash2flag(addr) == 0x0F0F)
		{
			Serial_SendByte(0xFF);
			Serial_SendByte(0xFF);
			Serial_SendByte(0xFF);
			Serial_SendByte(0xFF);
			W25Q64_ReadData(addr, height, 256);
			Serial_SendArray(height, 256);
			addr += 0x000100;
			
			
			Serial_SendByte(0xEE);
			Serial_SendByte(0xEE);
			Serial_SendByte(0xEE);
			Serial_SendByte(0xEE);
			W25Q64_ReadData(addr, accel, 256);
			Serial_SendArray(accel, 256);
			
			addr += 0x000100;
			ind ++;
		}
	
		OLED_ShowString(0, 0, "Send Done.", BIG);
		OLED_ShowString(0, 2, "Total:", BIG);
		OLED_ShowNum(57, 2, ind-1, 3, BIG);
}


int main(void)
{
			Key_Init();
			//Servo_Init();
			LED13_Init();
			//Beep_Init();
			OLED_Init();
			BMP280_Init();
			W25Q64_Init();
			Serial_Init();
			MPU6050_Init();
			//ShortLine_Init();
			OLED_Clear();
	
	
			OLED_ShowString(0, 0, "If send data to UART,", SML);
			OLED_ShowString(0, 1, "press Key plz.", SML);
			OLED_ShowString(0, 3, "Now:", SML);
			for(uint8_t i = 0; i < 31; i ++)
			{
					workModeFlag = Key_GetState();
					OLED_ShowNum(30, 3, workModeFlag, 1, BIG);
					Delay_ms(100);
				
					if(workModeFlag)
					{
							Data_UART2PC();
							return;
					}
			}
		
			Flash_ReadLast();Delay_s(5);
			OLED_Clear();
			Height_Flash_Control();
	
//			{
//						W25Q64_Init();
//						W25Q64_SectorErase(0x000000);
//			}

//			Self_Detect();
}

         
void TIM2_IRQHandler(void)	//TIM2的中断处理函数，产生更新中断时会自动执行
{		
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	//先检查中断标志位，获取TIM的更新中断标志位
		{
			d_second ++;
			
			ht2arr(BMP280_GetHeight(), height, htInd);
			htInd ++;
			
			if (d_second & 1)
			{
					ac2arr(MPU6050_CaculateAccel(&Data), accel, acInd);
					acInd ++;
			}
			
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//清除标志位
		}
}




