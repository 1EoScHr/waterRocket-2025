#ifndef __SERVO_H
#define __SERVO_H

void Servo_Init(void);	//ʹ��ǰ����PWM��Ҫ�����

void Servo1_SetAngle(float Angle);
void Servo2_SetAngle(float Angle);
void Servo3_SetAngle(float Angle);
void Para_relese(void);	//��ɡ
void Dlg_relese(void);		//�ͷ�DLG

#endif
