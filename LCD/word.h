/*
状态机文件

*/
#ifndef __WORD_H__
#define __WORD_H__

#include "stdint.h"
#include "font_header.h"


typedef struct WORD{
	const char *character;
	uint16_t x;
	uint16_t y;
	uint16_t color;
	uint16_t background;
	const BX_FONT *font;
	uint16_t valx;
	uint16_t valy;
}WORD;

void word_init(void);


extern const WORD *pt_word;





#endif
