#include "stm32f10x.h"
#include "LED.h"
#include <math.h>
#include "Beep.h"
#include "Timer.h"
#include "Delay.h"
#include "Servo.h"
#include "BMP280.h"
#include "W25Q64.h"
#include "MPU6050.h"
#include "ShortLine.h"
#include "OLED_Simple.h"

MPU6050_Data Data;

#define DATA_HEAD_FLAG 0x0F0F
uint8_t dummy[2];
uint8_t height[256];
uint32_t dd_second = 0;
uint8_t readhtflag = 0;

void ErrorHandle(void)
{
	LED13_Init();
	Beep_Init();

	LED13_on();
	Beep_JustLoud();

	while(1)
	{}
}

uint16_t flash2data(uint32_t addr)
{
		W25Q64_ReadData(addr, dummy, 2);
		return (dummy[0] << 8 | dummy[1]);
}

void data2flash(uint16_t u16, uint8_t* u8_arr, uint8_t ind)
{
		u8_arr[2*ind] = u16 >> 8; u8_arr[2*ind+1] = u16;
}

void Self_Detect(void) 	// Ӳ���Լ�
{
		uint8_t i = 3;
		
		LED13_Init();				// ����Ӳ����user led��beep
		Beep_Init();
		while(i)
		{
				LED13_on();
				Beep_JustLoud();
				Delay_ms(250);
				LED13_off();
				Beep_Stop();
				Delay_ms(250);
				i --;
		}
	
		Beep_JustLoud();LED13_on();Delay_ms(500);Beep_Stop();LED13_off();
		
		OLED_Init();				// ����Ӳ����OLED
		OLED_OpenShow();
		
		Beep_JustLoud();LED13_on();Delay_ms(500);Beep_Stop();LED13_off();
		
		ShortLine_Init();		// һ��Ӳ����shortline
		OLED_ShowString(0, 0, "cnct stLine plz", SML);
		while(!ShortLine())
		{}
		OLED_ShowString(0, 0, "plug", SML);
		while(ShortLine())
		{}
		OLED_Clear();
		
		Beep_JustLoud();LED13_on();Delay_s(1);Beep_Stop();LED13_off();
		
		BMP280_Init();			// ��ҪӲ������ѹ��
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
		OLED_ShowHexNum(0, 0, MPU6050_ReadID(), 3, BIG);
		
		if(BMP280_ReadID() != 0x68)
		{
				OLED_ShowString(72, 0, "Wrong!", BIG);
				LED13_on();Beep_JustLoud();
				while(1)
				{}
		}
		
}

void Height_Flash(void)	// �����и߶����ݼ�¼
{
		//----------------------------------------------��ʼ��
	
		OLED_Init();
		BMP280_Init();
		W25Q64_Init();
		ShortLine_Init();
	
		OLED_Clear();
		//W25Q64_SectorErase(0x000000);

		uint32_t addr_start = 0x000000;

		//----------------------------------------------ȷ����ʼ��ַ

		while (flash2data(addr_start) == DATA_HEAD_FLAG)
		{
			addr_start += 0x000100;
			// if (addr_start) // ����Ժ��ٴ���
		}
		
		OLED_ShowString(0, 0, "addr:", SML);
		OLED_ShowHexNum(31, 0, addr_start, 6, SML);

		//----------------------------------------------����׼��

		data2flash(0x0F0F, height, 0);

		int16_t height_begin;

		//----------------------------------------------������ʾ&��ʼ�߶�

		OLED_ShowString(0, 2, "cnct stline, plz", SML);
		while(!ShortLine())
		{}
		
		OLED_ShowString(0, 2, "plug stline, plz", SML);
		while (ShortLine())
		{
			height_begin = BMP280_GetHeight();
			OLED_ShowNum(16, 4, height_begin, 3, BIG);
		}
		
		OLED_ShowString(0, 2, "OK, thanks :)    ", SML);
		data2flash(height_begin, height, 1);
	
		//----------------------------------------------��ɺ�

		uint8_t ind = 2;
		Timer_Internal_Init();

		while(dd_second < 1200)
		{
				if(readhtflag)
				{
						data2flash(BMP280_GetHeight(), height, ind);
						readhtflag = 0;
						ind ++;
				}
		}
		
		W25Q64_PageProgram(addr_start, height, 256);
		OLED_ShowString(0, 6, "Done.", BIG);
}

void Flash_ReadLast(void)
{
		OLED_Init();
		OLED_Clear();
		
		uint32_t addr_emp = 0x000000;
		if(flash2data(addr_emp) != 0x0F0F)
		{
			OLED_ShowString(0, 0, "Error: No data in flash!", BIG);
			ErrorHandle();
		}

		uint32_t addr_use = addr_emp;
		addr_emp += 0x000100;

		while (flash2data(addr_emp) == 0x0F0F)
		{
			addr_use = addr_emp;
			addr_emp += 0x0001000;
		}
		
		W25Q64_ReadData(addr_use, height, 256);
		
		uint16_t hst = height[2]<<8 | height[3];
		OLED_ShowString(0, 0, "start:", BIG);
		OLED_ShowNum(57, 0, hst, 3, BIG);

		for(uint8_t ind = 2; ind < 128; ind ++)
		{
			if(hst <  height[2*ind]<<8 | height[2*ind+1])
			{
				hst = height[2*ind]<<8 | height[2*ind+1];
			}
		}

		OLED_ShowString(0, 2, "hmost:", BIG);
		OLED_ShowNum(57, 0, hst, 3, BIG);		
}

void TIM2_IRQHandler(void)	//ѡ��TIM���жϴ����������������ж�ʱ���Զ�ִ��
{		
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)	//�ȼ���жϱ�־λ����ȡTIM�ĸ����жϱ�־λ
		{
			dd_second ++;
			if (dd_second % 10 == 0)
			{
				readhtflag = 1;
			}
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	//�����־λ
		}
}
	
int main(void)
{
		//Self_Detect();
		Flash_Detect();
}

//int main(void)
// {
//			{/*//�����ø߶ȼƴ��벿��
//			OLED_Init(); OLED_OpenShow();	//��������
//			BMP280_Init();
//			W25Q64_Init();
//			Beep_Init();
//	 
//			OLED_ShowString(0, 0,"last:", BIG);
//			W25Q64_ReadData(0x000000, height, 2);
//			OLED_ShowNum(0, 2, (height[0] << 8) + height[1], 3, BIG);
//			Delay_s(10);
//			
//			Beep_JustLoud();
//			Delay_ms(750);
//			Beep_Stop();
//			Delay_ms(1000);
//			Beep_JustLoud();
//			Delay_ms(750);
//			Beep_Stop();
//			Delay_ms(1000);
//			Beep_JustLoud();
//			Delay_ms(750);
//			Beep_Stop();
//			
//			OLED_Clear();
//			
//			OLED_ShowString(0, 0, "now:", BIG);
//			OLED_ShowString(48, 0, "beg:", BIG);
//			OLED_ShowString(96, 0, "mst:", BIG);
//	
//			beginheight = BMP280_GetHeight();
//			OLED_ShowNum(48, 2, beginheight, 3, BIG);
//			OLED_ShowChar(72, 2, 'm', BIG);
//	
//			mostheight = beginheight;
//			OLED_ShowNum(96, 2, mostheight, 3, BIG);
//			OLED_ShowChar(120, 2, 'm', BIG);
//	
//			OLED_ShowChar(24, 2, 'm', BIG);
//			
//			while (1)
//			{
//			/////////////////////////////////////////////////////////////////////////////////////�߶���ʾ
//			nowheight=BMP280_GetHeight();
//			OLED_ShowNum(0, 2, nowheight, 3, BIG);

//			if (nowheight > mostheight)
//			{
//					mostheight = nowheight;
//					height[0] = mostheight >> 8;
//					height[1] = mostheight;
//					W25Q64_SectorErase(0x000000);
//					W25Q64_PageProgram(0x000000, height, 2);
//			}
//			
//			OLED_ShowNum(96, 2, mostheight, 3, BIG);
//			}
//			*/}
//			
//		{//ʵ�ʷɿز���
//			
//			/*---------------------------���Ӳ����ʼ��---------------------------*/
//		
//			ShortLine_Init();	//
//			LED13_Init();
//			Beep_Init();
//			Servo_Init();
//			BMP280_Init();
//			OLED_Init();
//			W25Q64_Init();
//			MPU6050_Init();
//				
//			/*------------------------------��ǹ���------------------------------*/
//			
//			Servo1_SetAngle(0);
//			Servo2_SetAngle(0);	
//			
//			/*------------------------------OLED����------------------------------*/
//			
//			OLED_OpenShow();	
//			
//			/*----------------------------��ʾ�ϴ�����----------------------------*/
//			
//			OLED_ShowString(0, 0,"last:", BIG);
//			W25Q64_ReadData(0x000000, height, 2);
//			OLED_ShowNum(0, 2, (height[0] << 8) + height[1], 3, BIG);
//			Delay_s(3);
//			OLED_Clear();
//			
//			/*----------------------------������ʾ׼��----------------------------*/
//			
//			OLED_ShowString(0, 0, "BEGa&h:", BIG);
//			OLED_ShowString(0, 2, "PLAN:", BIG);
//			OLED_ShowString(0, 4, "PARA:", BIG);
//			OLED_ShowString(0, 6, "MOST:", BIG);
//			
//				
//			/*---------------------��һ�γ��������̽������---------------------*/
//			
//			Beep_JustLoud();	
//			LED13_on();
//			Delay_ms(500);
//			while(ShortLine() == 0)	//δ�Ӷ̽���һֱ����
//			{
//					
//			}
//				
//			Beep_Stop();	//ֱ������ֹͣ
//			LED13_off();
//			
//			/*-----------------------�ڶ��ζ�����׼���÷���-----------------------*/
//			
//			while(ShortLine())
//			{
//					Beep_Stop();
//					Delay_ms(200);
//					LED13_on();
//					Beep_JustLoud();
//					Delay_ms(200);
//					LED13_off();
//					
//					if(ShortLine() == 0)
//					{
//							Delay_ms(50);	//�ʶ���ʱ����ֹ����̽���ʱ����
//					}	
//			}
//				
//			/*-------------------------------�����-------------------------------*/
//			
//			MPU6050_GetData(&Data);	//��ȡ����ʾ��ܼ��ٶ�����
//			OLED_ShowFloatNum(54, 0, MPU6050_CaculateAccel(&Data), 2, 3, SML); 
//			
//			OLED_ShowNum(54, 1, mostheight = BMP280_GetHeight(), 3, SML);	//��ʾ��ʼ�߶�
//			
//			for(int i = 0; i < 7; i ++)	//�׷ɻ�
//			{
//					LED13_on();
//					Delay_ms(500);
//					LED13_off();
//					nowheight = BMP280_GetHeight();
//					if(nowheight > mostheight)
//					{
//							OLED_ShowNum(48, 6, mostheight = nowheight, 3, BIG);
//					}
//			}
//			
//			Servo1_SetAngle(90);
//			OLED_ShowNum(48, 2, BMP280_GetHeight(), 3, BIG);	//��ʾ�׷ɻ��߶�
//			
//			
//			Delay_ms(500);	//��ɡ
//			Servo2_SetAngle(90);
//			OLED_ShowNum(48, 4, nowheight = BMP280_GetHeight(), 3, BIG);	//��ʾ��ɡ�߶�
//			
//			/*---------------------��ɺ��¼���߶ȱ��ڸĽ�---------------------*/
//			
//			Beep_Stop();
//			OLED_ShowNum(48, 6, mostheight, 3, BIG);
//			
//			while(1)
//			{
//					nowheight = BMP280_GetHeight();
//				
//					if(nowheight > mostheight)
//					{	
//							OLED_ShowNum(48, 6, mostheight = nowheight, 3, BIG);
//						
//							height[0] = mostheight >> 8;
//							height[1] = mostheight;
//							W25Q64_SectorErase(0x000000);
//							W25Q64_PageProgram(0x000000, height, 2);
//					}
//			}                                     
//			
//		}
//			                            
// }
//	                                            
                                                                                                                                                                                                                                                                   