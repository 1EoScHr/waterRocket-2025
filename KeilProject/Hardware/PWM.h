#ifndef __PWM_H
#define __PWM_H

void PWM_Init(void);

void PWM_SetCompare1(uint16_t Compare);	//在程序运行中修改CH1的CCR，以此修改占空比
void PWM_SetCompare2(uint16_t Compare);
void PWM_SetCompare3(uint16_t Compare);

#endif
