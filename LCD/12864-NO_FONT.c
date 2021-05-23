/*
注意：用这个小屏幕一定要关掉fsmc，否则读数据时会出现问题（当读取得数据是0xfx与0xex时，读出的数据不对）

原理：	一个片选64行（x地址，3bit，0~7），分为8页，其中每页其实是8行。这个x是不自增的。寄存器地址从 0xB8开始
		一个片选64列（y地址，6bit，0~63），这个y是自增的。寄存器地址从 0x40 开始
		所以一个ram地址是对应8*8bit数据内容
		控制器为 KS0108B
*/

#include "lcd_header.h" 
#include "stdint.h"

#if SCREEN == TM12864_NO_FONT


/*
引脚映射
PD14 -> RS	(D0)
PD0 -> R/W	(D2)
PE7 -> E	(D4)	这个引脚拉高再拉低，数据发送到RAM

PE9 -> DB0		(D6)
PE11 -> DB1		(D8)
PE13 -> DB2		(D10)
PE15 -> DB3		(D12)
PD9 -> DB4		(D14)
PG12 -> DB5		(FSMC_NE4)
PD5 -> DB6		(FSMC_NWE)
PG15 -> DB7		(TOUCH_RES#)
	
PG14 -> CS1		(I2C2_SCL)
PB1  -> CS2		(PWM)
PG13 -> RST		(I2C2_SDA)

电源与背光都接3v3
VO空，NC空，VOUT空
*/
#define CMD_TYPE(X)		(X & 0x1) ? Bit_SET : Bit_RESET

//控制引脚
#define RS(X)		X ? GPIO_SetBits(GPIOD, GPIO_Pin_14) : GPIO_ResetBits(GPIOD, GPIO_Pin_14)
#define RW(X)		X ? GPIO_SetBits(GPIOD, GPIO_Pin_0) :  GPIO_ResetBits(GPIOD, GPIO_Pin_0)      
#define EN(X)   	X ? GPIO_SetBits(GPIOE, GPIO_Pin_7) :  GPIO_ResetBits(GPIOE, GPIO_Pin_7) 

//片选
#define CS1(X)   	X ? GPIO_SetBits(GPIOG, GPIO_Pin_14) :  GPIO_ResetBits(GPIOG, GPIO_Pin_14) 
#define CS2(X)   	X ? GPIO_SetBits(GPIOB, GPIO_Pin_1) :  GPIO_ResetBits(GPIOB, GPIO_Pin_1) 

//数据总线写
#define DB0_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_9,  X);
#define DB1_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_11, X);
#define DB2_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_13, X);
#define DB3_W(X)		GPIO_WriteBit(GPIOE, GPIO_Pin_15, X);
#define DB4_W(X)		GPIO_WriteBit(GPIOD, GPIO_Pin_9,  X);
#define DB5_W(X)		GPIO_WriteBit(GPIOG, GPIO_Pin_12, X);
#define DB6_W(X)		GPIO_WriteBit(GPIOD, GPIO_Pin_5,  X);
#define DB7_W(X)		GPIO_WriteBit(GPIOG, GPIO_Pin_15, X);
//数据总线读
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
等待12684的忙状态结束
*/
static void wait_busy(void)
{
	uint32_t i = 0;
	dat_pin_float();
    RS(0);     			//RS = 0.
	RW(1);        		//RW = 1.
    EN(1);        		//EN = 1.
    while(GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15) && i++ < 10000);//DB7 的值为1则说明液晶屏在忙
    EN(0);      		//EN = 0;
    dat_pin_out();      //数据引脚设为推挽输出
}


/*
写命令
写指令寄存器
*/
void tm12864_WriteCmd(uint8_t cmd)
{
	wait_busy();            //等待液晶屏忙完
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

    EN(0);	//EN = 0
}


/*
写数据
*/
static void tm12864_WriteData(uint8_t  cmd)  
{
	wait_busy(); 
    RS(1);	//RS = 1. 
    RW(0);	//RW = 0. 
    EN(1);	//EN = 1.
	
	DB0_W(CMD_TYPE(cmd >> 0));
	DB1_W(CMD_TYPE(cmd >> 1));
	DB2_W(CMD_TYPE(cmd >> 2));
	DB3_W(CMD_TYPE(cmd >> 3));
	DB4_W(CMD_TYPE(cmd >> 4));
	DB5_W(CMD_TYPE(cmd >> 5));
	DB6_W(CMD_TYPE(cmd >> 6));
	DB7_W(CMD_TYPE(cmd >> 7));
	
	EN(0);	//EN = 0;
}

/*
读数据
*/
static uint8_t tm12864_ReadData(void)  
{
	uint8_t tmp;
	uint32_t i;
	wait_busy();
	dat_pin_float();	//设置为输入引脚
    RS(1);	//RS = 1. 
    RW(1);	//RW = 1. 
    EN(1);	//EN = 1.
	for(i = 0; i < 700; i++){};//这个等待要根据花屏情况来计算
	tmp = (DB0_R) | (DB1_R << 1) | (DB2_R << 2) | (DB3_R << 3) | (DB4_R << 4) | (DB5_R << 5) | (DB6_R << 6) | (DB7_R << 7); 
	EN(0);	//EN = 0;
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
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG | RCC_APB2Periph_GPIOB , ENABLE); 
	
	//CS2引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//初始化推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
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
	
	//复位屏幕
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);   //rst 设置为高电平
	delay_ms(500); 
	GPIO_SetBits(GPIOG, GPIO_Pin_13);     //rst 设置为高电平
}


/*
初始化 12864
*/
static void tm12864_init(void)
{			
	CS2(1);//片选cs2
	CS1(1);//片选cs1
	tm12864_WriteCmd(0x3e); //关显示
//	tm12864_WriteCmd(0x3f); //开显示
	tm12864_WriteCmd(0XC0 | 0);	//设置起始地址
	tm12864_WriteCmd(0x3f); //开显示
}


/*
屏幕方向设置
*/
void bsp_lcd_set_directrion(uint8_t type, uint16_t *x, uint16_t *y)
{
	
}


/*
lcd 硬件初始化
*/
void bsp_lcd_hard_congfigure(void)
{
	LCD_GPIO_Config();
	tm12864_init();
	
//	lcd_show_string("12", 2, 0, 1, 0, font16);
}

/*
驱动封装层获得物理参数
*/
uint16_t bsp_get_xsize(void)
{
	return 128;//横向坐标范围
}
uint16_t bsp_get_ysize(void)
{
	return 64;//纵向坐标范围
}


/********************************************************
按照屏幕内存分布刷屏 写DDRAM
显示划分
行分为8行，8bit一行
列分为16列，8bit一列
最基本单位 8*8 bit
********************************************************/

/*
显示16*8点阵
table_8 ：点阵数据
x：行
y：列
注意这里不是指坐标
*/
static uint8_t show_16x8(const uint8_t *table_8, uint8_t x, uint8_t y, bool type)
{
	uint8_t i;
	
	if(x > 7 || y > 128)//超过显示范围 x:0~15;y:0~128
		return FALSE;
	
	if(y < 64)//cs1,左半屏
	{
		CS2(0);
		CS1(1);
	}
	else//cs2，右半屏
	{
		CS1(0);
		CS2(1);
		y -= 64;
	}
	//每个16*8的显示占2页
	//显示第一页
	tm12864_WriteCmd(0xb8 | x);	//设置页地址（行地址）
	tm12864_WriteCmd(0x40 | y);	//设置列 Y地址（列地址）
	for(i =0; i < 8; i++)
	{
		if(type == 0)
			tm12864_WriteData(table_8[i]);//写数据
		else
			tm12864_WriteData(~table_8[i]);//写数据
	}
	//显示第二页
	tm12864_WriteCmd(0xb8 | (x + 1));	//设置页地址（行地址）
	tm12864_WriteCmd(0x40 | y);			//设置y地址（列地址）
	for(i = 8; i < 16; i++)
	{
		if(type == 0)
			tm12864_WriteData(table_8[i]);//写数据
		else
			tm12864_WriteData(~table_8[i]);//写数据
	}
	
	CS1(1);
	CS2(1);          
	tm12864_WriteCmd(0x3f); //开显示
	return TRUE;
}



/*
显示16*16点阵
参数：table_16：16*16点阵;x：字符所在行；y：字符所在列；type :0 普通显示，1 反白显示

x：行,纵坐标
y：列,横坐标
注意这里不是指坐标
*/
static uint8_t show_16x16(const uint8_t *table_16, uint8_t x, uint8_t y, bool type)
{
	uint8_t i, tmp;
	
	tmp = y;
	if(x > 7 || y > 128)//超过显示范围 x:0~15;y:0~128
		return FALSE;
	
	if(y < 64)//cs1,左半屏
	{
		CS2(0);
		CS1(1);
	}
	else//cs2，右半屏
	{
		CS1(0);
		CS2(1);
		tmp -= 64;
	}
	
	//写上半页
	tm12864_WriteCmd(0xb8 | x);	//设置页地址（行地址）
	tm12864_WriteCmd(0x40 | tmp);	//设置列 Y地址（列地址）
	for(i = 0; i < 16; i++) //显示字的上半部分
	{
		if(y+i == 64)//跨屏
		{
			CS2(1);            //右半屏
			CS1(0);
			tm12864_WriteCmd(0xb8 | x);
			tm12864_WriteCmd(0x40 | 0);    //设置显示的行  
		}
		else
		if( ((y+i) % 128) == 0)//跨屏
		{
			CS1(1);            //左半屏
			CS2(0);
			tm12864_WriteCmd(0xb8 | x);
			tm12864_WriteCmd(0x40 | 0);    //设置显示的行  
		}
		
		if(type == 0)
			tm12864_WriteData(table_16[i]);
		else
			tm12864_WriteData(~table_16[i]);
	}
	
	//写下半页
	tmp = y;

	if(y < 64)
	{
		CS1(1);            //左半屏
		CS2(0);
	}
	else
	{
		CS2(1);            //右半屏
		CS1(0);
		tmp -= 64;
	}
	tm12864_WriteCmd( 0xb8 | (x+1));
	tm12864_WriteCmd( 0X40 | tmp);    
	for(i = 16; i < 32; i++) //显示字的下半部分
	{
		if(y+i-16 == 64)
		{
			CS2(1);            //右半屏
			CS1(0);
			tm12864_WriteCmd(0xb8 | (x+1));
			tm12864_WriteCmd(0X40 | 0);    //设置显示的行
		}
		else
		if(((y+i-16)%128) == 0)
		{
			CS1(1);
			CS2(0);            //左半屏
			tm12864_WriteCmd(0xb8 | (x+1));
			tm12864_WriteCmd(0X40 | 0);    //设置显示的行
		}
		if(type == 0)
			tm12864_WriteData(table_16[i]);
		else
			tm12864_WriteData(~table_16[i]);
	}
	
	CS1(1);
	CS2(1);          
	tm12864_WriteCmd(0x3f); //开显示
	return TRUE;
}


/*********************************************************
对外接口函数
*********************************************************/


/*
清屏
*/
void lcd_fill_color(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
	uint8_t i,j;
		
	CS1(1);
	CS2(1);					//同时片选两个
    for (i = 0;i <8; i++)	//循环清零每页
    {
    	tm12864_WriteCmd(i | 0xB8);	//页地址 X
    	tm12864_WriteCmd(0 | 0x40);	//列地址 Y
        for (j = 0; j < 64; j++) //循环清零每列
        {
        	tm12864_WriteData(0x00);
        }
    }

}

/*
显示二维码
参数：x ,y 起始坐标
xlong 显示宽度 因为是方形，长宽相同
scale 放大系数
dat 点阵，一个字节表示一个点

注意：需要把每8个 dat数据合为一个字节写入ddram
*/
void show_ercode(uint16_t x, uint16_t y, uint16_t xylong, uint8_t scale , uint8_t dat[][41]) 
{
	uint8_t i, j ,k, tmp;
	
	//右半屏
	CS1(0);
	CS2(1);
	
	//此屏幕为竖着打点，1~40行
	for(i = 0; i < 5; i++)
	{
		tm12864_WriteCmd(0xb8 | i);	//设置页地址（行地址）
		tm12864_WriteCmd(0x40 | 87);//设置列 Y地址（列地址）
		for(j = 0; j < 41; j++)
		{
			tm12864_WriteCmd(0x40 | (87 + j));//设置列 Y地址（列地址）
			for(k = 0, tmp = 0; k < 8; k++)
			{
				tmp |= dat[k + i*8][j] << (k);
			}
			tm12864_WriteData(~tmp);
		}
	}
	//画最后一行的横线 第41行
	tm12864_WriteCmd(0xb8 | 5);	//设置页地址（行地址）
	tm12864_WriteCmd(0x40 | 87);//设置列 Y地址（列地址）
	for(j = 0; j < 41; j++)
	{
		tm12864_WriteCmd(0x40 | (87 + j));//设置列 Y地址（列地址）
		tm12864_WriteData(1);
	}
	
	CS1(1);
	CS2(1);          
	tm12864_WriteCmd(0x3f); //开显示
}

/*
字模点阵打印
参数：	lattice 点阵指针；xSize 字符宽度；Ysize 字符高度（无用）；x 横坐标；y 纵坐标；
		color 字体颜色（高亮与否）；background 字体背景色（无用）
*/
void lcd_print_lattic(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t forcground,uint16_t background)
{
	bool type = FALSE;
	if(forcground == BLACK)//高亮显示
		type = TRUE;
	
	//	x /= 8;//计算相对值	//这里横坐标就是按像素点的，所以不需除8
	if(xSize == 8)
	{
		show_16x8(lattice, y, x, type);
	}
	else
	if(xSize == 16)
	{
		show_16x16(lattice, y, x, type);
	}

}


#endif

