#ifndef __W25Q64_H
#define __W25Q64_H

/*内部函数*/
//void W25Q64_WriteEnable(void);	//写使能
//void W25Q64_WaitBuzy(void)	//等待结束Buzy模式

void W25Q64_Init(void);	//W25Q64初始化
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);	//指针读取厂商ID与设备ID
void W25Q64_PageProgram(uint32_t Address, uint8_t *OutDataArray, uint16_t CountMax256);	//指定页编程写入数据，地址、数组、数据量
void W25Q64_SectorErase(uint32_t Address);	//指定扇区擦除
void W25Q64_ReadData(uint32_t Address, uint8_t *InDataArray, uint32_t Count);	//指定地址读取数据

#endif
