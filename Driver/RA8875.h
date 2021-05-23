#include "stm32f10x.h"
typedef struct
{
  vu16 LCD_REG;
  vu16 LCD_RAM;
} LCD_TypeDef;


/* LCD is connected to the FSMC_Bank1_NOR/SRAM4 and NE4 is used as ship select signal */
#define LCD_BASE    ((u32)(0x6C000002 | 0x6C000000))   //��ַ�Ĵ���
#define LCD         ((LCD_TypeDef *) LCD_BASE)         //ָ��Ĵ���


//����LCD�ĳߴ�
#define LCD_WIDTH		800
#define LCD_HEIGHT	480

#define CURH0           0x46    /* Memory Write Cursor Horizontal Position Register 0 */
#define CURH1           0x47    /* Memory Write Cursor Horizontal Position Register 1 */
#define CURV0           0x48    /* Memory Write Cursor Vertical Position Register 0 */
#define CURV1           0x49    /* Memory Write Cursor Vertical Position Register 1 */
typedef struct
{
	uint16_t usFontCode;	        /* ������� 0 ��ʾ16���� */
	uint16_t usTextColor;	        /* ������ɫ */
	uint16_t usBackColor;	        /* ���ֱ�����ɫ��͸�� */
	uint16_t usSpace;		          /* ���ּ�࣬��λ = ���� */
}FONT_T;

enum
{
	FC_ST_16 = 0,		/* ����15x16���� ����x�ߣ� */
	FC_ST_12 = 1		/* ����12x12���� ����x�ߣ� */
};

enum
{
	RA_FONT_16 = 0,		/* RA8875 ���� 16���� */
	RA_FONT_24 = 1,		/* RA8875 ���� 24���� */
	RA_FONT_32 = 2		/* RA8875 ���� 32���� */
};
/*--------16λ��ɫֵ---------------*/
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	   0x001F  
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 //��ɫ
#define BRRED 			     0XFC07 //�غ�ɫ
#define GRAY  			     0X8430 //��ɫ
#define LGRAY 			     0XC618 //ǳ��ɫ

#define Line0          0
#define Line1          16
#define Line2          32
#define Line3          48
#define Line4          64
#define Line5          80
#define Line6          96
#define Line7          112
#define Line8          128
#define Line9          144
#define Line10         160
#define Line11         176
#define Line12         192
#define Line13         208
#define Line14         224

#define Horizontal     0x00
#define Vertical       0x01

void RA8875_OpenWindow(unsigned short x1,unsigned short x2,unsigned short y1,unsigned short y2);
void RA8875_Init(void);
void RA8875_Clear(u16 Color);
void Set_TextCursor(uint32_t x,uint32_t y);   //���Թ��
void Set_GraphicsCursor(uint32_t x,uint32_t y);  //�ڴ���
void RA8875_DispAscii(uint16_t x, uint16_t y, char *_ptr);
void RA8875_DispString(uint16_t X0, uint16_t Y0, char *_ptr, FONT_T *_tFont);
void GUI_Put(u16 x0, u16 y0, u8 *pcStr,u32 PenColor, u32 BackColor);
void RA8875_DrawBMP(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t *_ptr);
void RA8875_SetFrontColor(uint16_t Color);
void RA8875_SetBackColor(uint16_t Color);
void RA8875_FillRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color);
