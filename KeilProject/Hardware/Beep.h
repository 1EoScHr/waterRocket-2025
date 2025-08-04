#ifndef __BEEP_H
#define __BEEP_H

void Beep_Init(void);
void Beep_Ring(uint16_t PSC);	//在程序运行中改变PSC，改变频率即音调
void Beep_Stop(void);	//停止
void Beep_JustLoud(void);	//共振响度最大，2700Hz
	
extern const uint16_t Beep_C_mid[8];

#endif
