#include "stm32f10x.h"                  // Device header
#include "leoSPI.h"
#include "W25Q64_Ins.h"

/*使用W25Q64模块，只支持模式0与3，用模式0*/
/*扇区地址：xxx000-xxxFFF*/
/*页地址：xxxx00-xxxxFF*/

void W25Q64_Init(void)	//W25Q64初始化
{
		leoSPI_Init();
}

void W25Q64_WaitBuzy(void)	//等待结束Buzy模式
{
		uint32_t Timeout = 10000;	//防死循环
		
		leoSPI_Start();
		leoSPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);	//发送读状态寄存器1指令
		while ((leoSPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
		{
				Timeout --;
				if (Timeout == 0) break;
		}
		leoSPI_Stop();
}

void W25Q64_WriteEnable(void)	//写使能
{
		W25Q64_WaitBuzy();
		
		leoSPI_Start();	//起始
		leoSPI_SwapByte(W25Q64_WRITE_ENABLE);	//发送写使能指令，返回值无意义
		leoSPI_Stop();	//终止
}

void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)	//指针读取厂商ID与设备ID
{
		W25Q64_WaitBuzy();
		
		leoSPI_Start();	//起始
		leoSPI_SwapByte(W25Q64_JEDEC_ID);	//发送读ID指令，返回值无意义
		*MID = leoSPI_SwapByte(W25Q64_DUMMY_BYTE);	//接收MID，发送值无意义
		*DID = (leoSPI_SwapByte(W25Q64_DUMMY_BYTE) << 8);	//接收DID高八位
		*DID |= leoSPI_SwapByte(W25Q64_DUMMY_BYTE);	//接收DID低八位
		leoSPI_Stop();	//终止
}

void W25Q64_PageProgram(uint32_t Address, uint8_t *OutDataArray, uint16_t CountMax256)	//指定页编程写入数据，地址、数组、数据量
{
		W25Q64_WriteEnable();	//写使能;
	
		leoSPI_Start();
		leoSPI_SwapByte(W25Q64_PAGE_PROGRAM);	//发送页编程指令
		
		//连续发送三个地址
		leoSPI_SwapByte(Address >> 16);
		leoSPI_SwapByte(Address >> 8);
		leoSPI_SwapByte(Address);		
		
		//依次发送写入数据
		for (uint16_t i = 0; i < CountMax256; i ++)
		{
				leoSPI_SwapByte(OutDataArray[i]);	
		}
		
		leoSPI_Stop();
}

void W25Q64_SectorErase(uint32_t Address)	//指定扇区擦除
{
		W25Q64_WriteEnable();	//写使能
	
		leoSPI_Start();
		leoSPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);	
		
		//连续发送三个地址
		leoSPI_SwapByte(Address >> 16);
		leoSPI_SwapByte(Address >> 8);
		leoSPI_SwapByte(Address);		
		
		leoSPI_Stop();
}

void W25Q64_ReadData(uint32_t Address, uint8_t *InDataArray, uint32_t Count)	//指定地址读取数据
{
		W25Q64_WaitBuzy();
	
		leoSPI_Start();
		leoSPI_SwapByte(W25Q64_READ_DATA);	//发送读数据指令
		
		//连续发送三个地址
		leoSPI_SwapByte(Address >> 16);
		leoSPI_SwapByte(Address >> 8);
		leoSPI_SwapByte(Address);		
	
		//接收数据
		for (uint32_t i = 0; i < Count; i ++)
		{
				InDataArray[i] = leoSPI_SwapByte(W25Q64_DUMMY_BYTE);
		}	
		
		leoSPI_Stop();
}
