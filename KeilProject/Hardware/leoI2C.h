#ifndef __LEOI2C_H
#define __LEOI2C_H

//内部函数
//void leoI2C_W_SCL(uint8_t BitValue);	//写时钟线
//void leoI2C_W_SDA(uint8_t BitValue);	//写数据线
//uint8_t	leoI2C_R_SDA(void);	//读数据线

//基本函数
void leoI2C_Init(void);	//I2C初始化

void leoI2C_Start(void);	//起始条件
void leoI2C_Stop(void);	//终止条件
void leoI2C_SendByte(uint8_t Byte);	//发送一个字节
uint8_t leoI2C_ReceiveByte(void);	//接收一个字节
void leoI2C_SendAck(uint8_t AckBit);	//发送应答
uint8_t leoI2C_ReceiveAck(void);	//接收应答


#endif
