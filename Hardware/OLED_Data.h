#ifndef __OLED_DATA_H
#define __OLED_DATA_H

//ASCLL码表
extern const uint8_t OLED_F8x16[][16];	//8*16是因为取模顺序，分两行是因为16要分两页来写
extern const uint8_t OLED_F6x8[][6];

//图像数据
extern const uint8_t NWPU[];
extern const uint8_t xc[];
extern const uint8_t tk[];

#endif
