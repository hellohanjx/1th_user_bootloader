/*******************************************************************************
  * Company: Wang Electronic Technology Co., Ltd.
  ******************************************************************************
  * 文件名称：RA8875.c
  * 功能说明：红牛7寸LCD_RA8875驱动
  * 版    本：V1.0
	* 作    者：openmcu	
  * 日    期：2014-09-18
********************************************************************************
  * 文件名称：
  * 功能说明：
  * 版    本：
	* 更新作者:	
  * 日    期：
	* 更新内容：
********************************************************************************/

#include "stm32f10x.h"
#include "RA8875.h"
#include <stdio.h>
#include <string.h>

u16 POINT_COLOR=0x0000;	//画笔颜色
u16 BACK_COLOR=0xFFFF;  //背景色 

/* 保存当前显示窗口的位置和大小，这几个变量由 RA8875_SetDispWin() 进行设置 */
static __IO uint16_t s_WinX = 0;
static __IO uint16_t s_WinY = 0;
static __IO uint16_t s_WinHeight = 480;
static __IO uint16_t s_WinWidth = 800;

/**************************************************************************************
** Function name:       void LCD_Configuration(void)
** Descriptions:        LCD相关引脚配置
** input parameters:    无
** output parameters:   无
** Returned value:      无
***************************************************************************************/
void LCD_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                       RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                       RCC_APB2Periph_AFIO, ENABLE);
	
	/* Set  PD.00(D2), PD.01(D3), PD.04(NOE)--LCD_RD （读）, PD.05(NWE)--LCD_WR（写）,
	      PD.08(D13),PD.09(D14),PD.10(D15),PD.14(D0),PD.15(D1) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | 
								GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                //复用推挽输出
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
	
	/* Set PG.12 PE2 (LCD_CS) as alternate function push pull - CE3(LCD /CS) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	                 //复用推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOE, GPIO_Pin_2);
}

/**************************************************************************************
** Function name:       void LCD_FSMCConfiguration(void)
** Descriptions:        配置FSMC
** input parameters:    无
** output parameters:   无
** Returned value:      无
***************************************************************************************/
void LCD_FSMCConfiguration(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInitStructure;
	
	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 4 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 3;
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 4;
	FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;
	FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
	
	/* Color LCD configuration ------------------------------------
	LCD configured as follow:
	- Data/Address MUX = Disable
	- Memory Type = SRAM
	- Data Width = 16bit
	- Write Operation = Enable
	- Extended Mode = Enable
	- Asynchronous Wait = Disable */
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
	
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
	
	/* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

/**************************************************************************************
** Function name:       void LCD_Configuration(void)
** Descriptions:        delay
** input parameters:    cnt:0~65535
** output parameters:   无
** Returned value:      无
***************************************************************************************/		 
void delay(int cnt)
{
	volatile unsigned int dl;
	while(cnt--)
	{
		for(dl=0; dl<500; dl++);
	}
}
/**************************************************************************************
** Function name:       void RA8875_WriteCmd(unsigned char cmd)
** Descriptions:        向8875写命令
** input parameters:    cmd:Command
** output parameters:   无
** Returned value:      无
***************************************************************************************/
static void RA8875_WriteCmd(unsigned char cmd)
{
// 	if (ra8875_wait_busy() == 0)
// 	{
		LCD->LCD_REG = (unsigned short)cmd;
// 	}	
}

/**************************************************************************************
** Function name:       void RA8875_WriteData(unsigned short dat)
** Descriptions:        向8875写数据
** input parameters:    dat:Data
** output parameters:   无
** Returned value:      无
***************************************************************************************/
static void RA8875_WriteData(unsigned short dat)
{
	LCD->LCD_RAM = dat;	
}

/**************************************************************************************
** Function name:       unsigned char RA8875_ReadStatus(void)
** Descriptions:        读RA8875状态
** input parameters:    无
** output parameters:   无
** Returned value:      temp:寄存器状态
***************************************************************************************/
static unsigned char RA8875_ReadStatus(void)
{
	unsigned short temp = 0;
	temp = LCD->LCD_REG;
	return (unsigned char)temp;
}

/**************************************************************************************
** Function name:       unsigned char RA8875_WaitBusy(void)
** Descriptions:        忙等待
** input parameters:    无
** output parameters:   无
** Returned value:      0:忙  1:非忙
***************************************************************************************/
unsigned char RA8875_WaitBusy(void)
{
	unsigned int i = 50000;

	while (i--)
	{
		if ((GPIOD->IDR & GPIO_Pin_2) != (unsigned int)Bit_RESET)
		{
			return 0;
		}
	}
	return 1;
}

/**************************************************************************************
** Function name:       void RA8875_WriteReg(unsigned char reg,unsigned char value)
** Descriptions:        给指定寄存器写数据
** input parameters:    reg:寄存器  value:数据
** output parameters:   无
** Returned value:      无
***************************************************************************************/
 void RA8875_WriteReg(unsigned char reg,unsigned char value)
{
	if (RA8875_WaitBusy() == 0)
	{
		LCD->LCD_REG = (unsigned short)reg;
		LCD->LCD_RAM = (unsigned short)value;
	}	
}

/**************************************************************************************
** Function name:      unsigned char RA8875_ReadReg(unsigned char reg,unsigned char *value)
** Descriptions:       读指定寄存器数据
** input parameters:   reg:寄存器  value:数据
** output parameters:  数据地址
** Returned value:     无
***************************************************************************************/
static unsigned char RA8875_ReadReg(unsigned char reg,unsigned char *value)
{
	if (RA8875_WaitBusy() == 0)
	{
    LCD->LCD_REG = (unsigned short)reg;
		*value = (unsigned char)LCD->LCD_RAM;
		return 0;
	}
	else
	{
		return 1;
	}
}

/**************************************************************************************
** Function name:      unsigned char RA8875_readreg(unsigned char regadd)
** Descriptions:       读指定寄存器数据
** input parameters:   regadd:寄存器地址
** output parameters:  无
** Returned value:     读取的寄存器数据
***************************************************************************************/
static unsigned char RA8875_readreg(unsigned char regadd)
{
	uint8_t value;
  LCD->LCD_REG = value;
	value = (unsigned char)LCD->LCD_RAM;
	return value;
}

/**************************************************************************************
** Function name:      void RA8875_OpenWindow(unsigned short x1,unsigned short x2,unsigned short y1,unsigned short y2)
** Descriptions:       设置显示窗口
** input parameters:   x1:x轴起点 x2:x轴终点  y1:y轴起点 y2:y轴终点
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_OpenWindow(unsigned short x1,unsigned short x2,unsigned short y1,unsigned short y2)
{
	RA8875_WriteReg(0x30,(unsigned char)(x1));   //设置水平起点
	RA8875_WriteReg(0x31,(x1 >> 8));
	
	RA8875_WriteReg(0x34,(unsigned char)(x2));    //设置水平终点
	RA8875_WriteReg(0x35,(x2 >> 8));


	RA8875_WriteReg(0x32,(unsigned char)(y1));    //设置垂直起点
	RA8875_WriteReg(0x33,(y1 >> 8));

	RA8875_WriteReg(0x36,(unsigned char)(y2));     //设置垂直终点
	RA8875_WriteReg(0x37,(y2 >> 8));
}

/**************************************************************************************
** Function name:      void RA8875_Init(void)
** Descriptions:       RA8875控制器初始化
** input parameters:   无
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_Init(void)
{
	
	uint8_t reg,val;

 	LCD_Configuration();             
	LCD_FSMCConfiguration();
	RA8875_WriteReg(0x01,0x01);
	delay(10);
	
	RA8875_WriteReg(0x01,0xf0);
	delay(10);
		
	/*PLL init*/
	RA8875_WriteReg(0x88,0x0B);
	delay(5000);

	RA8875_WriteReg(0x89,0x02);
	delay(5000);
	
	reg = RA8875_ReadStatus();
	if (reg)
	{}
	else
	{
		reg = RA8875_ReadReg(0x88,&val);
		while(reg);
	}
	
	
	/*MCU interface*/
	RA8875_WriteReg(0x10,0x0F);	//16bit 65K color
	RA8875_WriteReg(0x04,0x81);
	delay(500);
	/*Horizontal set*/
	RA8875_WriteReg(0x14,0x63);
	RA8875_WriteReg(0x15,0x03);
	RA8875_WriteReg(0x16,0x03);
	RA8875_WriteReg(0x17,0x02);
	RA8875_WriteReg(0x18,0x00);
	/*Vertical set*/
  RA8875_WriteReg(0x19,0xDF);
	RA8875_WriteReg(0x1A,0x01);
	RA8875_WriteReg(0x1B,0x14);
	RA8875_WriteReg(0x1C,0x00);
	RA8875_WriteReg(0x1D,0x06);
	RA8875_WriteReg(0x1E,0x00);
	RA8875_WriteReg(0x1F,0x01);
	/*active window*/
	RA8875_OpenWindow(0,LCD_WIDTH - 1,0,LCD_HEIGHT - 1);

	delay(20);

		/*disp on*/
	RA8875_WriteReg(0x01,0x80);
	/*set RA8875 GPOX pin to 1 - disp panel on*/
	RA8875_WriteReg(0xC7,0x01);
}


/**************************************************************************************
** Function name:      void RA8875_Clear(u16 Color)
** Descriptions:       将屏幕填充成指定的颜色，如清屏，则填充 0xffff
** input parameters:   Color      填充值
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_Clear(u16 Color)
{
	int i=0;
		RA8875_WriteCmd(0x02);
	for(i=0;i<800*480;i++)
	{
		RA8875_WriteData(Color);
	}

}

/**************************************************************************************
** Function name:      Set_TextCursor(uint32_t x,uint32_t y)
** Descriptions:       设置光标
** input parameters:   x,y：屏幕坐标
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void Set_TextCursor(uint32_t x,uint32_t y)   //测试光标
{
	uint8_t temp;

	RA8875_ReadReg(0x40,&temp);

	temp |= (0x01 << 7);

	RA8875_WriteReg(0x40,temp);

	RA8875_WriteCmd(0x2A);
	RA8875_WriteData((uint8_t)x);
	RA8875_WriteCmd(0x2B);
	RA8875_WriteData((uint8_t)(x >> 8));

	RA8875_WriteCmd(0x2C);
	RA8875_WriteData((uint8_t)y);
	RA8875_WriteCmd(0x2D);
	RA8875_WriteData((uint8_t)(y >> 8));
	
}

/**************************************************************************************
** Function name:      Set_GraphicsCursor(uint32_t x,uint32_t y) 
** Descriptions:       设置光标
** input parameters:   x,y：屏幕坐标
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void Set_GraphicsCursor(uint32_t x,uint32_t y)  //内存光标
{
	RA8875_WriteReg(0x40,0x00);
	RA8875_WriteReg(0x41,0x00);
	
	RA8875_WriteCmd(CURH1);
	RA8875_WriteData(x>>8);
	RA8875_WriteCmd(CURH0);
	RA8875_WriteData(x);

	RA8875_WriteCmd(CURV1);
	RA8875_WriteData(y>>8);
	RA8875_WriteCmd(CURV0);
	RA8875_WriteData(y);	
	
}

/**************************************************************************************
** Function name:      void LCD_DrawPoint1(uint32_t x,uint32_t y,uint32_t color)
** Descriptions:       打点(指定颜色)
** input parameters:   x,y：屏幕坐标
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void LCD_DrawPoint1(uint32_t x,uint32_t y,uint32_t color)
{
	Set_GraphicsCursor(x,y);         //设置显存光标
	RA8875_WriteCmd(0x02);           //发送写命令
	RA8875_WriteData(color);         //开始写数据
}

/**************************************************************************************
** Function name:      void LCD_DrawPoint(uint32_t x,uint32_t y)
** Descriptions:       打点(默认颜色)
** input parameters:   x,y：屏幕坐标
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void LCD_DrawPoint(uint32_t x,uint32_t y)
{
	Set_GraphicsCursor(x,y);
	RA8875_WriteCmd(0x02);
	RA8875_WriteData(POINT_COLOR);
}

uint16_t LCD_GetPoint(uint16_t X,uint16_t Y)
{
	uint16_t Serial_Data;
	
 	RA8875_WriteCmd(0x41);
 	RA8875_WriteData(0x00);
	
	RA8875_WriteCmd(0x4a);
	RA8875_WriteData(X);   
	RA8875_WriteCmd(0x4b);	   
	RA8875_WriteData(X>>8);

	RA8875_WriteCmd(0x4c);
	RA8875_WriteData(Y);
	RA8875_WriteCmd(0x4d);	   
	RA8875_WriteData(Y>>8);
// 	
 //  delay(100);

   RA8875_WriteCmd(0x02);      //读两次，防止花屏
   Serial_Data=LCD->LCD_RAM;
   RA8875_WriteCmd(0x02);
   Serial_Data =  LCD->LCD_RAM;

//  	write_cmd(0x41);
//  	write_data(0x00);
  return Serial_Data;

}


/**************************************************************************************
** Function name:      void RA8875_DispAscii(uint16_t x, uint16_t y, char *_ptr)
** Descriptions:       显示ASCII字符
** input parameters:   x,y：屏幕坐标 _prt 字符串首地址
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_DispAscii(uint16_t x, uint16_t y, char *_ptr)
{
	Set_TextCursor(x, y);             //设置显示光标

	RA8875_WriteReg(0x40, (1 << 7));	// 设置为文本模式 

  RA8875_WriteReg(0x20, (1 << 3));	// 水平扫描方向设定
	RA8875_WriteReg(0x20, (1 << 2));	    // 垂直扫描方向设定
	
	//RA8875_WriteReg(0x22, (0 << 6));	    // 文字透明
  
	/* 选择CGROM font; 选择内部CGROM; 内部CGROM 编码选择ISO/IEC 8859-1. */
	RA8875_WriteReg(0x2F, 0x00);
	RA8875_WriteReg(0x21, (0 << 7) | (0 << 5) | (0 << 1) | (0 << 0));

  //RA8875_WriteReg(0x22, (1<<4));   //文字旋转90度
	 
   //   RA8875_WriteReg(0x22, (1 << 3) | (1 << 2) );   //水平放大4倍
 	 
   //	  RA8875_WriteReg(0x22, (1 << 1) | (1 << 0) );   //垂直放大4倍



	 RA8875_WriteCmd(0x02); 		//用于设定RA8875 进入内存(DDRAM或CGRAM)读取/写入模式 

	while (*_ptr != 0)
	{
	  RA8875_WriteData(*_ptr);
		while ((RA8875_ReadStatus() & 0x80) == 0x80);	// 必须等待内部写屏操作完成 
		_ptr++;
	}

	RA8875_WriteReg(0x40, (0 << 7));	// 还原为图形模式 
}

/**************************************************************************************
** Function name:      void RA8875_SetFrontColor(uint16_t Color)
** Descriptions:       设定前景色
** input parameters:   Color:颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_SetFrontColor(uint16_t Color)
{
//	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x63, (Color & 0xF800) >> 11);	// R5  
	RA8875_WriteReg(0x64, (Color & 0x07E0) >> 5);	// G6 
	RA8875_WriteReg(0x65, (Color & 0x001F));			// B5 
//	s_ucRA8875Busy = 0;
}

/**************************************************************************************
** Function name:      void RA8875_SetBackColor(uint16_t Color)
** Descriptions:       设定前景色
** input parameters:   Color:颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_SetBackColor(uint16_t Color)
{
//	s_ucRA8875Busy = 1;
	RA8875_WriteReg(0x60, (Color & 0xF800) >> 11);	// R5  
	RA8875_WriteReg(0x61, (Color & 0x07E0) >> 5);	// G6 
	RA8875_WriteReg(0x62, (Color & 0x001F));			// B5 
//	s_ucRA8875Busy = 0;
}

/**************************************************************************************
** Function name:      void RA8875_Layer1Visable(void)
** Descriptions:       RA8875 图层1显示打开
** input parameters:   无
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_Layer1Visable(void)
{

	RA8875_WriteReg(0x52, RA8875_readreg(0x52) & 0xF8);	//只有图层1 显示 
}


/**************************************************************************************
** Function name:      void RA8875_Layer2Visable(void)
** Descriptions:       只显示图层2
** input parameters:   无
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_Layer2Visable(void)
{

	RA8875_WriteReg(0x52, (RA8875_readreg(0x52) & 0xF8) | 0x01);	//只有图层2 显示 
}


/**************************************************************************************
** Function name:      void  RA8875_DrawLine(uint16_t X1 , uint16_t Y1 , uint16_t X2 , uint16_t Y2 , uint16_t _usColor)
** Descriptions:       采用RA8875的硬件绘图功能，在2点间画一条直线。
** input parameters:   X1, Y1 ：起始点X坐标
**			               X2, Y2 ：终止点Y坐标
**                     Color  : 颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_DrawLine(uint16_t X1 , uint16_t Y1 , uint16_t X2 , uint16_t Y2 , uint16_t Color)
{

	
	RA8875_WriteReg(0x91, X1);               //设置起点坐标 
	RA8875_WriteReg(0x92, X1 >> 8);
	RA8875_WriteReg(0x93, Y1);
	RA8875_WriteReg(0x94, Y1 >> 8);

	
	RA8875_WriteReg(0x95, X2);               //设置终点坐标 
	RA8875_WriteReg(0x96, X2 >> 8);
	RA8875_WriteReg(0x97, Y2);
	RA8875_WriteReg(0x98, Y2 >> 8);

	RA8875_SetFrontColor(Color);	            //设置直线的颜色 

  if (RA8875_WaitBusy() == 0)
	{
		RA8875_WriteReg(0x90, (1 << 7) | (0 << 4) | (0 << 0));	//开始画直线 
//	while (RA8875_ReadReg(0x90) & (1 << 7));				         //等待结束 
	
  }
	
}

/**************************************************************************************
** Function name:      void RA8875_DrawRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
** Descriptions:       RA8875硬件功能绘制矩形
** input parameters:   X,Y   ：矩形左上角的坐标
**		                 Height：矩形的高度
**		                 Width ：矩形的宽度
**                     Color : 颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_DrawRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
{
	
	RA8875_WriteReg(0x91, X);                   // 设置起点坐标 
	RA8875_WriteReg(0x92, X >> 8);
	RA8875_WriteReg(0x93, Y);
	RA8875_WriteReg(0x94, Y >> 8);

	
	RA8875_WriteReg(0x95,  X + Width - 1);        // 设置终点坐标 
	RA8875_WriteReg(0x96, (X + Width - 1) >> 8);
	RA8875_WriteReg(0x97,  Y + Height - 1);
	RA8875_WriteReg(0x98, (Y + Height - 1) >> 8);

	RA8875_SetFrontColor(Color);	                  //设置颜色 

  if(RA8875_WaitBusy() == 0)
	{
	  RA8875_WriteReg(0x90, (1 << 7) | (0 << 5) | (1 << 4) | (0 << 0));	 //开始画矩形 
//	while (RA8875_ReadReg(0x90) & (1 << 7));							             // 等待结束 
	} 
}

/**************************************************************************************
** Function name:      void RA8875_FillRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
** Descriptions:       RA8875硬件功能绘制矩形
** input parameters:   X,Y   ：矩形左上角的坐标
**		                 Height：矩形的高度
**		                 Width ：矩形的宽度
**                     Color : 填充颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_FillRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
{
	
	RA8875_WriteReg(0x91, X);                  // 设置起点坐标 
	RA8875_WriteReg(0x92, X >> 8);
	RA8875_WriteReg(0x93, Y);
	RA8875_WriteReg(0x94, Y >> 8);

	
	RA8875_WriteReg(0x95, X + Width - 1);          // 设置终点坐标 
	RA8875_WriteReg(0x96, (X + Width - 1) >> 8);
	RA8875_WriteReg(0x97, Y + Height - 1);
	RA8875_WriteReg(0x98, (Y + Height - 1) >> 8);

	RA8875_SetFrontColor(Color);	                   //设置颜色 

	RA8875_WriteReg(0x90, (1 << 7) | (1 << 5) | (1 << 4) | (0 << 0));	// 开始填充矩形 
//	while (RA8875_ReadReg(0x90) & (1 << 7));							          // 等待结束 

}

/**************************************************************************************
** Function name:      void RA8875_DrawCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
** Descriptions:       RA8875硬件功能绘制圆形
** input parameters:   X,Y   ：圆心坐标
**		                 Radius：半径
**                     Color : 图形颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_DrawCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
{
	/*
		RA8875 支持圆形绘图功能，让使用者以简易或低速的MCU 就可以在TFT 模块上画圆。先设定
		圆的中心点REG[99h~9Ch]，圆的半径REG[9Dh]，圆的颜色REG[63h~65h]，然后启动绘图
		REG[90h] Bit6 = 1，RA8875 就会将圆的图形写入DDRAM，相对的在TFT 模块上就可以显示所
		画的圆。若设定REG[90h] Bit5 = 1，则可画出一实心圆 (Fill)；若设定REG[90h] Bit5 = 0，则可
		画出空心圆 (Not Fill
	*/
	
	if (Radius > 255)               //设置圆的半径 
	{
		return;
	}

	
	RA8875_WriteReg(0x99, X);        //设置圆心坐标 
	RA8875_WriteReg(0x9A, X >> 8);
	RA8875_WriteReg(0x9B, Y);
	RA8875_WriteReg(0x9C, Y >> 8);

	RA8875_WriteReg(0x9D, Radius);	//设置圆的半径 

	RA8875_SetFrontColor(Color);	  // 设置颜色

	RA8875_WriteReg(0x90, (1 << 6) | (0 << 5));				//开始画圆, 不填充 
//	while (RA8875_ReadReg(0x90) & (1 << 6));				//等待结束 

}


/**************************************************************************************
** Function name:      void RA8875_FillCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
** Descriptions:       RA8875硬件功能绘制圆形
** input parameters:   X,Y   ：圆心坐标
**		                 Radius：半径
**                     Color : 填充颜色
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_FillCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
{
	if (Radius > 255)                     //设置圆的半径 
	{
		return;
	}

	
	RA8875_WriteReg(0x99, X);              // 设置圆心坐标 
	RA8875_WriteReg(0x9A, X >> 8);
	RA8875_WriteReg(0x9B, Y);
	RA8875_WriteReg(0x9C, Y >> 8);

	RA8875_WriteReg(0x9D, Radius);	                  // 设置圆的半径 

	RA8875_SetFrontColor(Color);	                   //  设置颜色 

	RA8875_WriteReg(0x90, (1 << 6) | (1 << 5));				// 开始画圆, 填充 
//	while (RA8875_ReadReg(0x90) & (1 << 6));				//等待结束 

}

/**************************************************************************************
** Function name:      void RA8875_SetDispWin(uint16_t X, uint16_t Y, uint16_t Height, uint16_t _usWidth)
** Descriptions:       设置显示窗口
** input parameters:   X,Y   ：显示坐标		                
**                     Height: 窗口高度
**		                 Width : 窗口宽度
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_SetDispWin(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width)
{

	uint16_t usTemp;


	RA8875_WriteReg(0x30, X);
  RA8875_WriteReg(0x31, X >> 8);

	RA8875_WriteReg(0x32, Y);
  RA8875_WriteReg(0x33, Y >> 8);

	usTemp = Width + X - 1;
	RA8875_WriteReg(0x34, usTemp);
  RA8875_WriteReg(0x35, usTemp >> 8);

	usTemp = Height + Y - 1;
	RA8875_WriteReg(0x36, usTemp);
  RA8875_WriteReg(0x37, usTemp >> 8);

	Set_GraphicsCursor(X, Y);

	s_WinX = X;
	s_WinY = Y;
	s_WinHeight = Height;
	s_WinWidth = Width;
}

/**************************************************************************************
** Function name:      void RA8875_QuitWinMode(void)
** Descriptions:       退出窗口显示模式，变为全屏显示模式
** input parameters:   无               
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_QuitWinMode(void)
{
	RA8875_SetDispWin(0, 0, 480, 800);
}

/**************************************************************************************
** Function name:      void RA8875_DrawBMP(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t *_ptr)
** Descriptions:       在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
** input parameters:   X, Y    : 图片的坐标
**		                 Height  ：图片高度
**		                 Width   ：图片宽度
**		                 _ptr    ：图片点阵指针              
** output parameters:  无
** Returned value:     无
***************************************************************************************/
void RA8875_DrawBMP(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t *_ptr)
{
	uint32_t index = 0;
	const uint16_t *p;
	RA8875_SetDispWin(X, Y, Height, Width);//设置图片的位置和大小， 即设置显示窗口

  RA8875_WriteReg(0x40, (0 << 7));	      // 设置为绘图模式 

	RA8875_WriteCmd(0x02); 		              // 准备读写显存 

	p = _ptr;
	for (index = 0; index < Height * Width; index++)
	{
		
		RA8875_WriteData(*p++);
	}

	RA8875_QuitWinMode();                     // 退出窗口绘图模式 
}
