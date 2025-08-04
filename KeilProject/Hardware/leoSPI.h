#ifndef __LEOSPI_H
#define __LEOSPI_H

/*内部函数*/
//void leoSPI_W_SS(uint8_t BitValue);	//写SS的引脚来从机选择，输出
//void leoSPI_W_SCK(uint8_t BitValue);	//写SCK的引脚来控制时钟，输出
//void leoSPI_W_MOSI(uint8_t BitValue);	//写MOSI的引脚来输出数据，输出
//uint8_t leoSPI_R_MISO(void);	//读MISO的引脚来读入数据，输入

void leoSPI_Init(void);	//SPI初始化
void leoSPI_Start(void);	//起始条件：SS从高电平置低电平
void leoSPI_Stop(void);	//终止条件：SS从低电平置高电平
uint8_t leoSPI_SwapByte(uint8_t ByteSend);	//模式0交换字节，用其他模式要对应更改

#endif
