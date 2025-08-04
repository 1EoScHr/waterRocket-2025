//int main(void)
// {
//			{/*//测试用高度计代码部分
//			OLED_Init(); OLED_OpenShow();	//开机动画
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
//			/////////////////////////////////////////////////////////////////////////////////////高度显示
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
//		{//实际飞控部分
//			
//			/*---------------------------相关硬件初始化---------------------------*/
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
//			/*------------------------------舵角归零------------------------------*/
//			
//			Servo1_SetAngle(0);
//			Servo2_SetAngle(0);	
//			
//			/*------------------------------OLED开屏------------------------------*/
//			
//			OLED_OpenShow();	
//			
//			/*----------------------------显示上次数据----------------------------*/
//			
//			OLED_ShowString(0, 0,"last:", BIG);
//			W25Q64_ReadData(0x000000, height, 2);
//			OLED_ShowNum(0, 2, (height[0] << 8) + height[1], 3, BIG);
//			Delay_s(3);
//			OLED_Clear();
//			
//			/*----------------------------做好显示准备----------------------------*/
//			
//			OLED_ShowString(0, 0, "BEGa&h:", BIG);
//			OLED_ShowString(0, 2, "PLAN:", BIG);
//			OLED_ShowString(0, 4, "PARA:", BIG);
//			OLED_ShowString(0, 6, "MOST:", BIG);
//			
//				
//			/*---------------------第一次长鸣：检测短接线情况---------------------*/
//			
//			Beep_JustLoud();	
//			LED13_on();
//			Delay_ms(500);
//			while(ShortLine() == 0)	//未接短接线一直长鸣
//			{
//					
//			}
//				
//			Beep_Stop();	//直到接上停止
//			LED13_off();
//			
//			/*-----------------------第二次短鸣：准备好发射-----------------------*/
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
//							Delay_ms(50);	//适度延时，防止插入短接线时抖动
//					}	
//			}
//				
//			/*-------------------------------发射后-------------------------------*/
//			
//			MPU6050_GetData(&Data);	//获取并显示离架加速度数据
//			OLED_ShowFloatNum(54, 0, MPU6050_CaculateAccel(&Data), 2, 3, SML); 
//			
//			OLED_ShowNum(54, 1, mostheight = BMP280_GetHeight(), 3, SML);	//显示初始高度
//			
//			for(int i = 0; i < 7; i ++)	//抛飞机
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
//			OLED_ShowNum(48, 2, BMP280_GetHeight(), 3, BIG);	//显示抛飞机高度
//			
//			
//			Delay_ms(500);	//开伞
//			Servo2_SetAngle(90);
//			OLED_ShowNum(48, 4, nowheight = BMP280_GetHeight(), 3, BIG);	//显示开伞高度
//			
//			/*---------------------完成后记录最大高度便于改进---------------------*/
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
            