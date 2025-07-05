#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/*舵机的协议规定，PWM周期要为20ms，高电平0.5ms为0°，高电平2.5ms为180°*/
/*根据协议设置PWM的参数，这里ARR+1=20000，PSC+1=72，CCR初始为0*/
/*根据计算可有：频率=50(周期即为20ms)，占空比=CCR/20000，分辨率=1/20000*/


void Servo_Init(void)	//使用前对照PWM的要求更改
{
			PWM_Init();
}

void Servo1_SetAngle(float Angle)
{
			//角度在0-180，据协议CCR应在500-2500，有换算关系：
			PWM_SetCompare2(Angle / 180 *2000 + 500);
}

void Servo2_SetAngle(float Angle)
{
			//角度在0-180，据协议CCR应在500-2500，有换算关系：
			PWM_SetCompare3(Angle / 180 *2000 + 500);
}

void Servo3_SetAngle(float Angle)
{
			//角度在0-180，据协议CCR应在500-2500，有换算关系：
			PWM_SetCompare1(Angle / 180 *2000 + 500);
}
