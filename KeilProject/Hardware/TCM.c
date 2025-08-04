#include "stm32f10x.h"                  // Device header
#include "OLED_Simple.h"
#include <math.h>

#define ALPHA 0.07
#define GGG   9.79136

//extern uint8_t height[256];
uint16_t Height[128] = {3855, 681, 682, 683, 683, 685, 686, 688, 690, 693, 696, 699, 702, 706, 711, 715, 719, 724, 728, 731, 735, 739, 743, 746, 750, 753, 756, 760, 763, 765, 768, 770, 773, 775, 777, 779, 781, 783, 784, 786, 787, 789, 790, 791, 792, 793, 794, 795, 795, 796, 796, 797, 797, 797, 797, 797, 797, 797, 796, 796, 795, 794, 794, 793, 792, 791, 790, 788, 787, 785, 784, 782, 780, 779, 777, 776, 774, 771, 769, 767, 767, 765, 764, 763, 761, 761, 760, 760, 760, 759, 759, 759, 758, 758, 757, 757, 756, 756, 756, 755, 755, 754, 754, 754, 754, 753, 753, 752, 752, 752, 751, 751, 751, 750, 750, 750, 749, 749, 749, 748, 748, 748, 0, 0, 0, 0, 0, 0};

static inline float det(float p, float q, float r,
												float s, float t, float u, 
												float v, float w, float x)
{
		return p*(t*x-u*w) - q*(s*x-u*v) + r*(s*w-t*v);
}	


// �̶����ݽ��㣬10��ֻ����0.8s���ٶ�ʮ�ֿɹ�
void TCM_noAir(void)	// Trajectory Calculation Module, �켣����ģ��
{
		/*
				ʹ��1s��ĸ߶����ݽ�����С������ϣ���Ϊ�۲�߶ȣ����ִ�Լǰ1s�Ĳ����ܺõĹ���������������
				�۲���ؿ��Է���ԭ����Ϊ����Ķ�����Լ��1.1s-1.3s�Ž���������Ҫ����������߾���Ҫ��ʧ�ؿ�ʼ��
		*/
		
		uint8_t startInd = 14;
		uint8_t S0 = 10;	// �ݶ�pointNum10���㣬Ҳ����1s�ڵ����������
		
		// �����ǹ̶���ʱ�䣬Ϊ�˽�ʡ������ֱ�ӽ��⼸��Ӳ����
	
		float S1 = 18.5; // 1.4 + ... + 2.3
		float S2 = 35.05;	// 1.4^2 + ... + 2.3^2
		float S3 = 67.895;	
		float S4 = 134.1973;
		
		//
		float H0 = 0, H1 = 0, H2 = 0, t = 1.4, h;
		for(uint8_t i = startInd; i < 24; i ++)	// �Ժ�Ҫ�ĳ������壬̫�鷳��
		{
					//h = ((height[2*i] << 8) + height[2*i+1]);
					h = Height[i];
					H0 += h;
					H1 += h * t;
					H2 += h * t * t;
					t += 0.1;
		}

		//
		float det_A = det(S4, S3, S2, 
											S3, S2, S1,
											S2, S1, S0);
		float det_a = det(H2, S3, S2, 
											H1, S2, S1,
											H0, S1, S0);		
		float det_b = det(S4, H2, S2, 
											S3, H1, S1,
											S2, H0, S0);		
		float det_c = det(S4, S3, H2, 
											S3, S2, H1,
											S2, S1, H0);	

		//
		float a = det_a / det_A;
		float b = det_b	/ det_A;
		float c = det_c / det_A;
		
		OLED_ShowFloatNum(0, 0, a, 3, 2, BIG);
		OLED_ShowFloatNum(0, 2, b, 3, 2, BIG);
		OLED_ShowFloatNum(0, 4, c, 3, 2, BIG);
		
}

// ��������Ŀ���ٶȴﵽ��ʱ��
float time_for_velocity(float vy0, float v_target) {
    float term = (v_target +9.81 / 0.15) / (vy0 + 9.81 / 0.15);
    if (term <= 0) return -1; // �޽�򳬳���Χ
    return -logf(term) / 0.15;
}

// ���㵽����ߵ�ʱ��
float time_to_apex(float vy0) {
    return logf(1.0f + (0.15 * vy0) / 9.81) / 0.15;
}

float y_future(float t, float y0, float vy0) {
    return y0 + (1.0f/0.15) * (vy0 + 9.81/0.15) * (1.0f - expf(-0.15 * t)) - (9.81/0.15) * t;
}

void TCM_oneAir(void)
{
		uint8_t startInd = 14;
		uint8_t S0 = 10;	// �ݶ�pointNum10���㣬Ҳ����1s�ڵ����������
		float S_f = 0, S_f2 = 0, S_y = 0, S_fy = 0;
		float t = 1.4;
    for (int i = startInd; i < 24; i++) {
        float f = 1.0f - expf(-ALPHA * t);            // f(t_i)
        float y_adj = Height[i] + (GGG / ALPHA) * t;         // ���������

        S_f  += f;
        S_f2 += f * f;
        S_y  += y_adj;
        S_fy += f * y_adj;
			
				t += 0.1;
    }

    // ���淽����� y0 �� C
    float denom = 10 * S_f2 - S_f * S_f;
    float C  = (10 * S_fy - S_f * S_y) / denom;
    
		float y0      = (S_y - C * S_f) / 10;

    // ���� vy0
    float vy0 = ALPHA * C - (GGG / ALPHA);
		
		OLED_ShowFloatNum(0, 0, y0, 3, 2, BIG);
		OLED_ShowFloatNum(0, 2, vy0, 3, 2, BIG);
}


