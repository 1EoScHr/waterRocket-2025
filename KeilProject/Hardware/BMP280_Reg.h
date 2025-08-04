#ifndef __BMP280_REG_H
#define __BMP280_REG_H

//一般BMP280芯片地址为 1110 11 0/1 0/1 （6*固定地址  1*可变地址  1*读写位，默认写）
#define BMP280_ADDRESS											 0xEC		//应读出从设备地址值

/*基本数据寄存器*/
#define BMP280_CHIPID_REG                    0xD0  /*ID寄存器 */
#define BMP280_RESET_REG                     0xE0  /*复位寄存器*/
#define BMP280_STATUS_REG                    0xF3  /*状态寄存器*/
#define BMP280_CTRLMEAS_REG                  0xF4  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG                    0xF5  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG              0xF7  /*Pressure 高位 Register */
#define BMP280_PRESSURE_LSB_REG              0xF8  /*Pressure 低位 Register */
#define BMP280_PRESSURE_XLSB_REG             0xF9  /*Pressure 小数位 Register */
#define BMP280_TEMPERATURE_MSB_REG           0xFA  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG           0xFB  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG          0xFC  /*Temperature XLSB Reg */

/*特定值*/
#define BMP280_RESET_VALUE									 0xB6		//写入复位寄存器
#define	BMP280_MEASURING										 0x01		//状态寄存器值：读取中
#define	BMP280_IM_UPDATE										 0x08		//状态寄存器值：更新中

/*矫正数据寄存器*/
#define BMP280_DIG_T1_LSB_REG                0x88
#define BMP280_DIG_T1_MSB_REG                0x89
#define BMP280_DIG_T2_LSB_REG                0x8A
#define BMP280_DIG_T2_MSB_REG                0x8B
#define BMP280_DIG_T3_LSB_REG                0x8C
#define BMP280_DIG_T3_MSB_REG                0x8D
#define BMP280_DIG_P1_LSB_REG                0x8E
#define BMP280_DIG_P1_MSB_REG                0x8F
#define BMP280_DIG_P2_LSB_REG                0x90
#define BMP280_DIG_P2_MSB_REG                0x91
#define BMP280_DIG_P3_LSB_REG                0x92
#define BMP280_DIG_P3_MSB_REG                0x93
#define BMP280_DIG_P4_LSB_REG                0x94
#define BMP280_DIG_P4_MSB_REG                0x95
#define BMP280_DIG_P5_LSB_REG                0x96
#define BMP280_DIG_P5_MSB_REG                0x97
#define BMP280_DIG_P6_LSB_REG                0x98
#define BMP280_DIG_P6_MSB_REG                0x99
#define BMP280_DIG_P7_LSB_REG                0x9A
#define BMP280_DIG_P7_MSB_REG                0x9B
#define BMP280_DIG_P8_LSB_REG                0x9C
#define BMP280_DIG_P8_MSB_REG                0x9D
#define BMP280_DIG_P9_LSB_REG                0x9E
#define BMP280_DIG_P9_MSB_REG                0x9F

#endif
