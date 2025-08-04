#ifndef __MPU6050_H
#define __MPU6050_H

typedef struct 
{
			int16_t AX;
			int16_t AY;
			int16_t AZ;		//加速度
			int16_t GX;
			int16_t GY;
			int16_t GZ;	//陀螺仪
}	MPU6050_Data;

//内部函数
//void MPU6050_WriteByte2Reg(uint8_t RegAddress, uint8_t Byte);
//uint8_t MPU6050_ReadByte2Reg(uint8_t RegAddress);

////void MPU6050_WriteData2Reg(uint8_t RegAddress, uint8_t Data);	//暂未完成	
////uint8_t MPU6050_ReadData2Reg(uint8_t RegAddress);

void MPU6050_Init(void);	//初始化
uint8_t MPU6050_ReadID(void);  //读取硬件id
void MPU6050_GetData(MPU6050_Data* Data);	//获取传感器数据
float MPU6050_CaculateAccel(MPU6050_Data *Data);	//

#endif
