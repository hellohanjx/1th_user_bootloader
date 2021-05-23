/*
带字库蓝屏
屏幕写满第一行，接着后面的写第3行
这个破屏幕，搞了整天才搞明白，比彩屏还麻烦
注意：用字库的话只能显示4行，并且位置不能变化
所以采用点阵来显示汉字
注意GDRAM的地址对应屏幕的相对关系

注意：用这个小屏幕一定要关掉fsmc，否则读数据时会出现问题（当读取得数据是0xfx与0xex时，读出的数据不对）
*/


#include "stdint.h"
#include "stm32f10x_gpio.h"
#include "lcd_header.h"

#if SCREEN == TM12864Z


/*
DDRAM地址
*/
const uint8_t AC_TABLE[] = {
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
};

/*
引脚映射
PD14 -> RS	(D0)
PD0 -> R/W	(D2)
PE7 -> E	(D4)

PE9 -> DB0		(D6)
PE11 -> DB1		(D8)
PE13 -> DB2		(D10)
PE15 -> DB3		(D12)
PD9 -> DB4		(D14)
PG12 -> DB5		(FSMC_NE4)
PD5 -> DB6		(FSMC_NWE)
PG15 -> DB7		(TOUCH_RES#)
	
PG14 -> PSB		(I2C2_SCL)
PG13 -> RST		(I2C2_SDA)

电源与背光都接3v3
VO空，NC空，VOUT空
*/
#define CMD_TYPE(X)		(X & 0x1) ? Bit_SET : Bit_RESET

#define RS(X)		X ? GPIO_SetBits(GPIOD, GPIO_Pin_14) : GPIO_ResetBits(GPIOD, GPIO_Pin_14)
#define RW(X)		X ? GPIO_SetBits(GPIOD, GPIO_Pin_0) :  GPIO_ResetBits(GPIOD, GPIO_Pin_0)      
#define EN(X)   	X ? GPIO_SetBits(GPIOE, GPIO_Pin_7) :  GPIO_ResetBits(GPIOE, GPIO_Pin_7) 

#define DB0_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_9,  X);
#define DB1_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_11, X);
#define DB2_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_13, X);
#define DB3_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_15, X);
#define DB4_W(X)		GPIO_WriteBit(GPIOD, GPIO_Pin_9,  X);
#define DB5_W(X)		GPIO_WriteBit(GPIOG, GPIO_Pin_12, X);
#define DB6_W(X)		GPIO_WriteBit(GPIOD, GPIO_Pin_5,  X);
#define DB7_W(X)		GPIO_WriteBit(GPIOG, GPIO_Pin_15, X);

#define DB0_R		 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9)
#define DB1_R		 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)
#define DB2_R		 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)
#define DB3_R		 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15)
#define DB4_R		 GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_9)
#define DB5_R		 GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_12)
#define DB6_R		 GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_5)
#define DB7_R		 GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15)

/*
设置数据引脚为浮空输入
*/
static void dat_pin_float (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/*
设置数据引脚为推挽输出
*/
static void dat_pin_out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
}


 
/*
等待12768的忙状态结束
*/
static void wait_busy(void)
{
	uint32_t i = 0;
	dat_pin_float();	//数据引脚定义为浮空输入
    RS(0);     			//RS = 0.
	RW(1);        		//RW = 1.
    EN(1);        		//EN = 1.
    while(GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15) && i++ < 100);//DB7 的值为1则说明液晶屏在忙
    EN(0);      		//EN = 0;
    dat_pin_out();      //数据引脚设为推挽输出
}


/*
写命令
写指令寄存器
*/
void tm12864_WriteCmd(uint8_t cmd)
{
	wait_busy();                                         //等待液晶屏忙完
    RS(0);      //RS = 0.
    RW(0);      //RW = 0. 
    EN(1);		//EN = 1.

	DB0_W(CMD_TYPE(cmd >> 0));
	DB1_W(CMD_TYPE(cmd >> 1));
	DB2_W(CMD_TYPE(cmd >> 2));
	DB3_W(CMD_TYPE(cmd >> 3));
	DB4_W(CMD_TYPE(cmd >> 4));
	DB5_W(CMD_TYPE(cmd >> 5));
	DB6_W(CMD_TYPE(cmd >> 6));
	DB7_W(CMD_TYPE(cmd >> 7));
		
//	for(i = 0; i < 30; i++){};//等待10us
    EN(0);	//EN = 0
//	for(i = 0; i < 30; i++){};//等待10us
}


/*
写数据函数
*/
static void tm12864_WriteData(uint8_t  cmd)  
{
	wait_busy(); 
    RS(1);	//RS = 1. 
    RW(0);	//RW = 0. 
//	for(i = 0; i < 10; i++){};
    EN(1);	//EN = 1.

	DB0_W(CMD_TYPE(cmd >> 0));
	DB1_W(CMD_TYPE(cmd >> 1));
	DB2_W(CMD_TYPE(cmd >> 2));
	DB3_W(CMD_TYPE(cmd >> 3));
	DB4_W(CMD_TYPE(cmd >> 4));
	DB5_W(CMD_TYPE(cmd >> 5));
	DB6_W(CMD_TYPE(cmd >> 6));
	DB7_W(CMD_TYPE(cmd >> 7));

//	for(i = 0; i < 30; i++){};//等待10us
	EN(0);	//EN = 0;
//	for(i = 0; i < 30; i++){};//等待10us
}

/*
读数据函数
*/
static uint8_t tm12864_ReadData(void)  
{
	uint32_t i = 0;
	uint8_t tmp;
	
	wait_busy();
	dat_pin_float();	//设置为输入引脚
//	OSTimeDly(1);//
	for(i = 0; i < 1000; i++){};//这里需要等待
    RS(1);	//RS = 1. 
    RW(1);	//RW = 1. 
	for(i = 0; i < 10; i++){};
    EN(1);	//EN = 1.
	for(i = 0; i < 30; i++){};
	tmp = (DB0_R) | (DB1_R << 1) | (DB2_R << 2) | (DB3_R << 3) | (DB4_R << 4) | (DB5_R << 5) | (DB6_R << 6) | (DB7_R << 7); 
	for(i = 0; i < 30; i++){};
	EN(0);	//EN = 0;
	for(i = 0; i < 30; i++){};
	return tmp;
}


 /*
 引脚配置
 */
static void LCD_GPIO_Config(void)
{
    
	GPIO_InitTypeDef GPIO_InitStructure;
	
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG , DISABLE);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE); //关闭fsmc时钟 
//	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, DISABLE); 
//	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM2);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_0 | GPIO_Pin_9 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//初始化推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//初始化推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//初始化推挽输出
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOG, GPIO_Pin_14);     //PSB 设置为高电平。并口模式
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);     //rst 设置为高电平
	delay_ms(100); 
	GPIO_SetBits(GPIOG, GPIO_Pin_13);     //rst 设置为高电平
//    GPIO_SetBits(GPIOE, GPIO_Pin_7);                    //EN = 1.
}


/*
初始化 12864
*/
static void tm12864_init(void)
{			
	
	delay_ms(2); 
	tm12864_WriteCmd(0x30); //8位并行，基础指令
    delay_ms(2); 
	tm12864_WriteCmd(0x03);	//AC归0（ DDRAM address）,不改变DDRAM内容；光标归零
    delay_ms(2); 
	tm12864_WriteCmd(0x0C); //显示ON，游标OFF，游标闪烁OFF
    delay_ms(2); 
    tm12864_WriteCmd(0x01); //清屏,AC = 0
    delay_ms(2); 
    tm12864_WriteCmd(0x06); //写入时,游标右移动，AC + 1
    delay_ms(2); 
    tm12864_WriteCmd(0x80); //DDRAM 地址计数器 AC = 0
    delay_ms(2); 

}

/*
写字符串
*/
void DisStr(char *s, uint8_t x, uint8_t y)
{
	tm12864_WriteCmd(AC_TABLE[8*x + y]);	//起始位置
	while(*s != '\0')
    {
        tm12864_WriteData(*s);
        s++;
        delay_ms(2);                                   
    }
}
/*
打印数字
*/
void DisStrNum(uint32_t num, uint8_t x, uint8_t y)
{
	char tmp[10];
	char *s;
	s = tmp;
	sprintf(tmp, "%u", num);
	
	tm12864_WriteCmd(AC_TABLE[8*x + y]);	//起始位置
	while(*s != '\0')
    {
        tm12864_WriteData(*s);
        s++;
        delay_ms(2);                                   
    }
}

/*
打印2位数字
*/
void DisStrNum_02(uint32_t num, uint8_t x, uint8_t y)
{
	char tmp[10];
	char *s;
	s = tmp;
	sprintf(tmp, "%02u", num);
	
	tm12864_WriteCmd(AC_TABLE[8*x + y]);	//起始位置
	while(*s != '\0')
    {
        tm12864_WriteData(*s);
        s++;
        delay_ms(2);                                   
    }
}

/*
屏幕方向设置
*/
void bsp_lcd_set_directrion(uint8_t type, uint16_t *x, uint16_t *y)
{
	
}

void lcd_memory_init(void);


/*
lcd 硬件初始化
*/
void bsp_lcd_hard_congfigure(void)
{
	LCD_GPIO_Config();
	tm12864_init();
}

/*
驱动封装层获得物理参数
*/
uint16_t bsp_get_xsize(void)
{
	return 128;//横向范围
}
uint16_t bsp_get_ysize(void)
{
	return 64;//纵向范围
}


/*
显示游标
*/
void cusor_show(void)
{
	tm12864_WriteCmd(0x30);	//基础指令集
	tm12864_WriteCmd(0x0c); //显示游标,游标位反白
}


/*
字模点阵打印
参数：
*/
void lcd_print_lattic(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t forcground,uint16_t background)
{

}

/*
字模点阵打印
参数：
*/
void print_lattice(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t color,uint16_t background)
{ 
	
}

void bsp_lcd_block_select(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	
}

void lcd_fill_color(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
}
#endif

