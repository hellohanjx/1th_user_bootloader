#ifndef __ILI9325_H__
#define __ILI9325_H__

#include "stdint.h"
#include "font_header.h"
#include "stm32f10x_gpio.h"
#include "word.h"

#define SCREENILI9806	0
#define SCREENRA8875	1
#define TM12864Z		2
#define TM12864_NO_FONT	3

#define SCREEN 			TM12864_NO_FONT 


/*
16bit 565 颜色
*/
#define   BLACK                0x0000                // ??:    0,   0,   0 //
#define   BLUE                 0x001F                // ??:    0,   0, 255 //
#define   GREEN                0x07E0                // ??:    0, 255,   0 //
#define   CYAN                 0x07FF                // ??:    0, 255, 255 //
#define   RED                  0xF800                // ??:  255,   0,   0 //
#define   MAGENTA              0xF81F                // ??:  255,   0, 255 //
#define   YELLOW               0xFFE0                // ??:  255, 255, 0   //
#define   WHITE                0xFFFF                // ??:  255, 255, 255 //
#define   NAVY                 0x000F                // ???:  0,   0, 128 //
#define   DGREEN               0x03E0                // ???:  0, 128,   0 //
#define   DCYAN                0x03EF                // ???:  0, 128, 128 //
#define   MAROON               0x7800                // ???:128,   0,   0 //
#define   PURPLE               0x780F                // ??:  128,   0, 128 //
#define   OLIVE                0x7BE0                // ???:128, 128,   0 //
#define   LGRAY                0xC618                // ???:192, 192, 192 //
#define   DGRAY                0x7BEF                // ???:128, 128, 128 //

#define		RGB_GRAY		0xcccccc	//????
#define		RGB_DGRAY		0x999999	//???


void  lcd_show_string(const char* str,uint16_t x ,uint16_t y, uint16_t foreground, uint16_t background, const BX_FONT font);
uint16_t transfer24Bit_to_16Bit(uint32_t color);
void  lcd_show_num(uint32_t num,uint16_t x,uint16_t y ,uint16_t foreground, uint16_t background, const BX_FONT font ,uint8_t type, uint8_t size) ;
void lcd_show_time(const WORD *time);
void lcd_clear(void);
void  lcd_show_character(const WORD *character);
void  lcd_clear_character(const WORD *character);
void lcd_init(void);
void lcd_print_lattic(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t forcground,uint16_t background);
void lcd_fill_color(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color);


void DisStr(char *s, uint8_t x, uint8_t y);
void DisStrNum(uint32_t num, uint8_t x, uint8_t y);
void DisStrNum_02(uint32_t num, uint8_t x, uint8_t y);



#endif


