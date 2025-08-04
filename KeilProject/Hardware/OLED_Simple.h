#include "OLED_Data.h"

#ifndef __OLED_SIMPLE_H
#define __OLED_SIMPLE_H

//宏定义
#define BIG				8
#define SML				6

//内部函数
//void OLED_WrtCmd(uint8_t Cmd);	//写命令
//void OLED_WrtData(uint8_t Data);	//写数据
//uint32_t OLED_Pow(uint32_t X, uint32_t Y);	

//基本函数
void OLED_Init(void);	//初始化
void OLED_Clear(void);
void OLED_SetCursor(uint8_t X, uint8_t Page);	//设置光标

//运用函数
void OLED_ShowChar(uint8_t X, uint8_t Page, char Char, uint8_t Size);
void OLED_ShowString(uint8_t X, uint8_t Page, char* String, uint8_t Size);
void OLED_ShowNum(uint8_t X, uint8_t Page, uint32_t Num, uint8_t Length, uint8_t Size);	//显示整数
void OLED_ShowSignedNum(uint8_t X, uint8_t Page, int32_t Num, uint8_t Length, uint8_t Size);	//显示带符号整数
void OLED_ShowFloatNum(uint8_t X, uint8_t Page, double Num, uint8_t IntLength, uint8_t FraLength, uint8_t Size);	//显示带符号小数
void OLED_ShowHexNum(uint8_t X, uint8_t Page, uint32_t Num, uint8_t Length, uint8_t Size);	//显示十六进制数
void OLED_ShowImage(uint8_t X, uint8_t Page, uint8_t Width, uint8_t HeightByPage, const uint8_t* Image);	//显示图像
void OLED_OpenShow(void);	//开机动画

#endif
