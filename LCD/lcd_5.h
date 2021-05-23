#ifndef __ILI9325_H__
#define __ILI9325_H__
#include "stm32f10x.h"

#define   BLACK                0x0000                // 黑色：    0,   0,   0 //
#define   BLUE                 0x001F                // 蓝色：    0,   0, 255 //
#define   GREEN                0x07E0                // 绿色：    0, 255,   0 //
#define   CYAN                 0x07FF                // 青色：    0, 255, 255 //
#define   RED                  0xF800                // 红色：  255,   0,   0 //
#define   MAGENTA              0xF81F                // 品红：  255,   0, 255 //
#define   YELLOW               0xFFE0                // 黄色：  255, 255, 0   //
#define   WHITE                0xFFFF                // 白色：  255, 255, 255 //
#define   NAVY                 0x000F                // 深蓝色：  0,   0, 128 //
#define   DGREEN               0x03E0                // 深绿色：  0, 128,   0 //
#define   DCYAN                0x03EF                // 深青色：  0, 128, 128 //
#define   MAROON               0x7800                // 深红色：128,   0,   0 //
#define   PURPLE               0x780F                // 紫色：  128,   0, 128 //
#define   OLIVE                0x7BE0                // 橄榄绿：128, 128,   0 //
#define   LGRAY                0xC618                // 灰白色：192, 192, 192 //
#define   DGRAY                0x7BEF                // 深灰色：128, 128, 128 //

//硬件相关的子函数
#define Bank1_LCD_D    ((uint32_t)0x6C000002)    //Disp Data ADDR
#define Bank1_LCD_C    ((uint32_t)0x6C000000)	   //Disp Reg ADDR


#define Lcd_Light_ON   GPIO_SetBits(GPIOG, GPIO_Pin_14 )		 	//GPIOD->BSRR = GPIO_Pin_3;
#define Lcd_Light_OFF  GPIO_ResetBits(GPIOG, GPIO_Pin_14 )			//GPIOD->BRR  = GPIO_Pin_3;



//Lcd初始化及其低级控制函数
void Lcd_Configuration(void);
void Lcd_Initialize(void);
void LCD_WR_REG(uint16_t Index,uint16_t CongfigTemp);
//Lcd高级控制函数

void Lcd_ColorBox(uint16_t x,uint16_t y,uint16_t xLong,uint16_t yLong,uint16_t Color);
void DrawPixel(uint16_t x, uint16_t y, int Color);
 uint16_t GetPoint( uint16_t x, uint16_t y);
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
void LCD_Fill_Pic(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const unsigned char* pic);
void LCD_ShowString(const char * zi,uint16_t x, uint16_t y ,uint16_t color,uint8_t fontSize);
void LCD_ShowNum(uint32_t value, uint8_t num, uint8_t type, uint16_t x, uint16_t y, uint16_t color, uint8_t fontSize);

#endif


