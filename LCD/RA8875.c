/*
RA8875 旺宝7寸屏
采用绘图模式，没有用到文字模式
估计所谓文字模式要结合ROM字库来用？？？有时间研究下
这个绘图流程一般是：
1：选定区域
2：打点（按照设定的扫描方式，和内存写入方向）

*/


#include "lcd_header.h" 



#if SCREEN == SCREENRA8875

/*
定义屏幕分辨率
*/

/*
命令寄存器与数据寄存器地址
*/
#define RA8875_REG    ((uint32_t)0x6C000002)    //Disp Data ADDR
#define RA8875_RAM    ((uint32_t)0x6C000000)	  //Disp Reg ADDR



static volatile uint16_t X_SIZE	= 800;
static volatile uint16_t Y_SIZE	= 480;
static volatile uint8_t  RA8875Busy = 0;//这个作为信号量来使用
static volatile uint16_t s_WinX = 0;
static volatile uint16_t s_WinY = 0;
static volatile uint16_t s_WinHeight = 480;
static volatile uint16_t s_WinWidth = 800;
static volatile uint8_t lcdDirection = 0;


 


/*
写命令
写指令寄存器
*/
static void RA8875_WriteCmd(uint8_t RegAddr)
{
	*(__IO uint16_t *)RA8875_REG = RegAddr;
}
/*
写数据
写数据寄存器
*/
static void RA8875_WriteData(uint8_t RegValue)
{
	*(__IO uint16_t *)RA8875_RAM = RegValue;	     
}

/*
写ra8875寄存器
*/
static void RA8875_WriteReg(uint8_t RegAddr, uint8_t RegValue)
{

	RA8875Busy = 1;
	*(__IO uint16_t *)RA8875_REG = RegAddr;	 //设置寄存器地址
	*(__IO uint16_t *)RA8875_RAM = RegValue;	 //写入寄存器值
	RA8875Busy = 0;

}


/*
设置写显存的光标位置
参数：x,y 起点坐标
注意 0x80 ~ 0x83 是光标图形的坐标
*/
static void RA8875_SetCursor(uint16_t X, uint16_t Y)
{
	
	RA8875_WriteReg(0x46, X);		//内存写入光标水平位置[7:0]
	RA8875_WriteReg(0x47, X >> 8);	//内存写入光标水平位置[9:8]
	RA8875_WriteReg(0x48, Y);		//内存写入光标垂直位置[7:0]
	RA8875_WriteReg(0x49, Y >> 8);	//内存写入光标垂直位置[9:8]
}

/*
选定操作区域
注意 Xend，Yend 并不是结束坐标，而是选定区域从Xstart，Ystart（起始坐标）开始的长度，是长度不是坐标
*/
void bsp_lcd_block_select(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	uint16_t Temp;
	if(lcdDirection)//竖屏时，交换xy值，交换高和宽
	{
		Temp = Xstart;
		Xstart = Ystart;
		Ystart = Temp;
		Temp = Yend;
		Yend = Xend;
		Xend = Temp;
	}
	RA8875_WriteReg(0x30, Xstart);		//工作窗口水平起始点[7:0]
	RA8875_WriteReg(0x31, Xstart >> 8);	//工作窗口水平起始点[9:8]

	RA8875_WriteReg(0x32, Ystart);		//工作窗口垂直起始点[7:0]
	RA8875_WriteReg(0x33, Ystart >> 8);	//工作窗口垂直起始点[9:8]

	Temp = Xend + Xstart - 1;				
	RA8875_WriteReg(0x34, Temp);		//工作窗口水平结束点[7:0]
	RA8875_WriteReg(0x35, Temp >> 8);	//工作窗口水平结束点[9:8]

	Temp = Yend + Ystart - 1;
	RA8875_WriteReg(0x36, Temp);		//工作窗口垂直结束点[7:0]
	RA8875_WriteReg(0x37, Temp >> 8);	//工作窗口垂直结束点[9:8]

	RA8875_SetCursor(Xstart, Ystart);	//设置内存光标写入位置（要与工作窗口起始位置匹配）
	
	RA8875_WriteCmd(0x02); //开始向内存写数据

	/* 
	保存当前窗口信息，提高单色填充效率；
	还可以读取0x30-0x37寄存器获得当前窗口，但是效率低
	*/
	s_WinX = Xstart;
	s_WinY = Ystart;
	s_WinHeight = Yend;
	s_WinWidth = Xend;	
}




 /*
 引脚配置
 */
static void LCD_GPIO_Config(void)
{
    
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                       RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                       RCC_APB2Periph_AFIO, ENABLE);
	
	/* Set  PD.00(D2), PD.01(D3), PD.04(NOE)--LCD_RD £¨¶Á£©, PD.05(NWE)--LCD_WR£¨Ð´£©,
	      PD.08(D13),PD.09(D14),PD.10(D15),PD.14(D0),PD.15(D1) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | 
								GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                //¸´ÓÃÍÆÍìÊä³ö
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) as alternate function push pull */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
	                            GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* Set PF.00(A0 (RS)) as alternate function push pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
  /* Set PG.12(NE4 (LCD/CS)) as alternate function push pull - CE3(LCD /CS) */
  /* NE4 configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/*
fmc配置
 */
static void LCD_FSMC_Config(void)
{
	FSMC_NORSRAMInitTypeDef  Init;
	FSMC_NORSRAMTimingInitTypeDef  Time;
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, DISABLE);
	delay_ms(100);
	FSMC_NORSRAMDeInit(FSMC_Bank1_NORSRAM4);
	delay_ms(100);
	Init.FSMC_ReadWriteTimingStruct = &Time;
	Init.FSMC_WriteTimingStruct = &Time;
	FSMC_NORSRAMStructInit(&Init);
	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 4 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	Time.FSMC_AddressSetupTime = 1;
	Time.FSMC_AddressHoldTime = 1;
	Time.FSMC_DataSetupTime = 2;
	Time.FSMC_BusTurnAroundDuration = 0;
	Time.FSMC_CLKDivision = 0;
	Time.FSMC_DataLatency = 0;
	Time.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	Init.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	Init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	Init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	Init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	Init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	Init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	
	Init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	Init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	Init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	Init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	Init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	Init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	Init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInit(&Init);

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}


/*
初始化 RA8875驱动芯片
*/
static void RA8875_InitHard(void)
{			
	RA8875_WriteCmd(0x88);		//PLL设定
	delay_ms(2);
	RA8875_WriteData(12);		//PLLDIVM [7] = 0，PLLDIVN [4:0] = 12

	delay_ms(2);

	RA8875_WriteCmd(0x89);
	delay_ms(2);
	RA8875_WriteData(2);		//PLLDIVK[2:0] = 2, ³ýÒÔ4 

	delay_ms(2);		//89h 与 88h 被设定后需要等待[锁屏时间](为保证 PLL 稳定) < 100us
	
	RA8875_WriteReg(0x10, (1 <<3 ) | (1 << 1));	//配置未16位mcu，16位tft接口（65k色）
	RA8875_WriteReg(0x04, 0x81);	//PDAT在PCLK下降沿被取样（就是时钟下降沿接收数据），PCLK时钟2倍频系统时钟频率（）
	delay_ms(2);	

	RA8875_WriteReg(0x14, 0x63);	//水平显示区域宽度[6:0](像素 = (0x63 + 1) * 8)
	RA8875_WriteReg(0x15, 0x00);	//DE 信号极性（high 动作），水平非显示期间微调宽度[3:0]
	RA8875_WriteReg(0x16, 0x03);	//水平非显示期间宽度设定位[4:0]
	RA8875_WriteReg(0x17, 0x03);	//水平同步信号起始地址宽度[4:0]
	RA8875_WriteReg(0x18, 0x0B);	//水平同步信号脉波宽度[4:0]
	RA8875_WriteReg(0x19, 0xDF);	//垂直显示区域高度[7:0],(注意：这个在公式中并没有x8)
	RA8875_WriteReg(0x1A, 0x01);	//垂直显示区域高度[8]
	RA8875_WriteReg(0x1B, 0x1F);	//垂直非显示期间设定位[7:0]
	RA8875_WriteReg(0x1C, 0x00);	//垂直非显示期间设定位[8]
	RA8875_WriteReg(0x1D, 0x16);	//垂直同步信号起始位[7:0]
	RA8875_WriteReg(0x1E, 0x00);	//垂直同步信号起始位[8]
	RA8875_WriteReg(0x1F, 0x01);	//垂直信号动作校准（low动作），垂直同步信号脉波宽度[6:0]
	RA8875_WriteReg(0xC7, 0x01);	//从gpix脚位输入数据，（应该是从mcu向ra8875写数据模式，而不是相反）
	RA8875_WriteReg(0x01, 0x80);	//lcd 开显示
	
	RA8875_WriteReg(0x41, 0x00);	//显示模式->图形光标1关闭，写入图层1（>480*400时选择图层1）
//	RA8875_WriteReg(0x20, (1 << 2));//垂直扫描方向（COM(n-1) 到 COM0）
//	RA8875_WriteReg(0x20, 0x00);	//水平和垂直扫描方向：水平SEG0 到 SEG(n-1)；垂直 COM0 到 COM(n-1)。  
}


/*
屏幕方向设置
*/
void bsp_lcd_set_directrion(uint8_t type, uint16_t *x, uint16_t *y)
{
	if(type == 1)//竖屏
	{
		lcdDirection = 1;
//		RA8875_WriteReg(0x20, 1 << 3);	//垂直扫描方向:COM(n-1) 到 COM0,水平扫描方向：SEG0 到 SEG(n-1)
//		RA8875_WriteReg(0x40, 1 );		//选择绘图模式，左->右，上->下
		RA8875_WriteReg(0x20, 0x00);	// 水平和垂直扫描方向设定
		RA8875_WriteReg(0x40, 0x09);	// 选择绘图模式  从下到上然后从左到右。	
		X_SIZE	= 480;
		Y_SIZE	= 800;
	}
	else//横屏
	{
		/*
		两个方向的横屏
		*/
		lcdDirection = 0;
		RA8875_WriteReg(0x40, 0x01);	//显示模式->绘图模式，光标不显示，游标不闪烁，内存写入方向（左->右，上->下）当内存读取光标时位置不自动+1
		RA8875_WriteReg(0x20, (1 << 3));//单图层;水平扫描方向（右到左？SEG(n-1) 到 SEG0）;垂直扫描方向（COM0 到 COM(n-1) )
//		RA8875_WriteReg(0x20, 0x04);	    // 水平和垂直扫描方向设定
//		RA8875_WriteReg(0x40, 0x01);	// 选择绘图模式   从左到右然后从上到下	
		X_SIZE	= 800;
		Y_SIZE	= 480;
	}
	*x = X_SIZE;
	*y = Y_SIZE;

}




/*
获取点信息
*/
uint16_t bsp_lcd_get_pixel( uint16_t x, uint16_t y)
{

	return 0;
}






/*
指定坐标打点
*/
void DrawPixel(uint16_t x, uint16_t y, int Color)
{
	bsp_lcd_block_select(x,x,y,y);
	*(__IO uint16_t *) (RA8875_REG) = Color;
}


/*
lcd 硬件初始化
*/
void bsp_lcd_hard_congfigure(void)
{
	LCD_GPIO_Config();
	LCD_FSMC_Config();
	delay_ms(10);
	RA8875_InitHard();
}







volatile uint16_t *bsp_get_reg(void)
{
	return (__IO uint16_t *) (RA8875_REG);
}
volatile uint16_t *bsp_get_ram(void)
{
	return (__IO uint16_t *) (RA8875_RAM);
}
uint16_t bsp_get_xsize(void)
{
	return X_SIZE;
}
uint16_t bsp_get_ysize(void)
{
	return Y_SIZE;
}











////////////////////////////////////////////////////////////////////////////////////////////////




/*
打印字模点阵
*/
void lcd_print_lattic(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t forcground,uint16_t background)
{
	uint32_t i;
	uint8_t curByte;//???????
	uint8_t curBit;	//???????
	
	bsp_lcd_block_select(x, xSize, y, ySize);//????
	curBit = 1;
	
//	RA8875Busy = 1;
	for(i = 0; i < xSize*ySize; i++)
	{
		curByte = lattice[i >> 3];//?????(??? i/8)
		curBit = (curBit << 7) | (curBit >> 1);
		
		if(curBit & curByte)
			*(__IO uint16_t *)RA8875_RAM = forcground;
		else
			*(__IO uint16_t *)RA8875_RAM  = background;
	}
//	RA8875Busy = 0;
}


/*
打印字模点阵
每个字节代表8个点
*/
void print_lattice(const uint8_t *lattice,uint32_t xSize,uint32_t ySize,uint16_t x,uint16_t y,uint16_t color,uint16_t background)
{ 
	uint32_t i ,num; //±éàúμ??óí?×??ú
	uint8_t curByte;
	uint8_t curBit;
	bsp_lcd_block_select(x, xSize ,y, ySize);//???¨??óò
	curBit=1;
	num = xSize*ySize;
	
	for(i = 0;i < num; i++)
	{
		curByte=lattice[i>>3];//1??×??ú8??BIT ?ùò?òa3yò?8 μèD§óúóòò?3???￡
		//??????êμê???curBit ?-?·óòò?1???￡
		curBit=(curBit<<7)|(curBit>>1);
		if(curBit & curByte)
			*(__IO uint16_t *)RA8875_RAM = color;
		else
			*(__IO uint16_t *)RA8875_RAM  = background;
	}
}

/*============================================
打印图片,每个字节代表一个点的颜色
============================================*/
/*
填充8位图片
*/
void LCD_Fill_Pic8(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const uint8_t* pic)
{
	uint32_t i, j, num;
	num = pic_H*pic_V*2;
	
	bsp_lcd_block_select(x,x+pic_H-1,y,y+pic_V-1);
	
	for (i = 0; i < num; i+=2)
	{
		j=pic[i];
		j=j<<8;
		j=j+pic[i+1];
		*(__IO uint16_t *)RA8875_RAM = j;
	}
}


/*
填充16位图片
*/
void LCD_Fill_Pic16(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const uint16_t *pic)
{
	uint32_t  i, num;
	
	bsp_lcd_block_select(x, x+pic_H-1, y, y+pic_V-1);
	num = pic_H*pic_V; 
	
	for (i = 0; i < num; i++)
	{
		*(__IO uint16_t *)RA8875_RAM = pic[i];
	}
}

/*
填充32位图片
*/
void LCD_Fill_Pic32(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const uint32_t *pic)
{
	uint32_t  i, num;
	bsp_lcd_block_select(x,x+pic_H-1,y,y+pic_V-1);
	num = pic_H*pic_V; 
	
	for (i = 0; i < num; i++)
	{
		*(__IO uint16_t *)RA8875_RAM = transfer24Bit_to_16Bit(pic[i]);
	}
}


/*
ì?3????¨??óò
*/
void lcd_fill_color(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
	uint32_t index = 0;
	
	bsp_lcd_block_select(xStart, xLong, yStart, yLong );//????

//	RA8875Busy = 1;

	for (index = 0; index < yLong * xLong; index++)
	{
		*(__IO uint16_t *)RA8875_RAM = (Color);
	}
//	RA8875Busy = 0;
}

/*
显示二维码
参数：x ,y 起始坐标
xlong 显示宽度 因为是方形，长宽相同
scale 放大系数
*/
void show_ercode(uint16_t x, uint16_t y, uint16_t xylong, uint8_t scale , uint8_t dat[][41]) 
{
	uint32_t i, j ,num;

	num = xylong*scale;//点阵宽度
	bsp_lcd_block_select(x, (num), y , (num) );
	
	for(i = 0; i < num; i++)
	{
		for(j = 0; j < num; j++)
		{
			*(__IO uint16_t *)RA8875_RAM = dat[i/scale][j/scale]? BLUE:WHITE;
		}
			 
	}
}


#endif

