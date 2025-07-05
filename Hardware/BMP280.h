#ifndef __BMP280_H
#define __BMP280_H

/*内部函数*/
//void BMP280_WriteByte2Reg(uint8_t RegAddress, uint8_t Byte);
//uint8_t BMP280_ReadByte2Reg(uint8_t RegAddress);

/*中间函数*/
//void BMP280_WaitRead(void);	//等待转换完成以读取数据
//double BMP280_TempCorrect(int32_t adc_T);	//返回摄氏度
//double BMP280_PresCorrect(int32_t adc_P);	//返回Pa
	

void BMP280_Init(void);	//BMP280初始化
//double BMP280_GetTemperature(void);	//获取寄存器内的原始温度数据并修正转换为准确值
void BMP280_GetData(double *Temp, double *Pres);	//获取寄存器内的原始气压数据并修正转换为准确值
uint8_t BMP280_ReadID(void);
uint16_t BMP280_GetHeight(void);

typedef struct  
{
	/* T1~P9 为补偿系数 */
	uint16_t T1;
	int16_t	T2;
	int16_t	T3;
	uint16_t P1;
	int16_t	P2;
	int16_t	P3;
	int16_t	P4;
	int16_t	P5;
	int16_t	P6;
	int16_t	P7;
	int16_t	P8;
	int16_t	P9;
} BMP280;

#endif
