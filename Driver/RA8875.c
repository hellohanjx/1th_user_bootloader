/*******************************************************************************
  * Company: Wang Electronic Technology Co., Ltd.
  ******************************************************************************
  * �ļ����ƣ�RA8875.c
  * ����˵������ţ7��LCD_RA8875����
  * ��    ����V1.0
	* ��    �ߣ�openmcu	
  * ��    �ڣ�2014-09-18
********************************************************************************
  * �ļ����ƣ�
  * ����˵����
  * ��    ����
	* ��������:	
  * ��    �ڣ�
	* �������ݣ�
********************************************************************************/

#include "stm32f10x.h"
#include "RA8875.h"
#include <stdio.h>
#include <string.h>

u16 POINT_COLOR=0x0000;	//������ɫ
u16 BACK_COLOR=0xFFFF;  //����ɫ 

/* ���浱ǰ��ʾ���ڵ�λ�úʹ�С���⼸�������� RA8875_SetDispWin() �������� */
static __IO uint16_t s_WinX = 0;
static __IO uint16_t s_WinY = 0;
static __IO uint16_t s_WinHeight = 480;
static __IO uint16_t s_WinWidth = 800;

/**************************************************************************************
** Function name:       void LCD_Configuration(void)
** Descriptions:        LCD�����������
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
***************************************************************************************/
void LCD_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                       RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                       RCC_APB2Periph_AFIO, ENABLE);
	
	/* Set  PD.00(D2), PD.01(D3), PD.04(NOE)--LCD_RD ������, PD.05(NWE)--LCD_WR��д��,
	      PD.08(D13),PD.09(D14),PD.10(D15),PD.14(D0),PD.15(D1) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | 
								GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                //�����������
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	                 //�����������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOE, GPIO_Pin_2);
}

/**************************************************************************************
** Function name:       void LCD_FSMCConfiguration(void)
** Descriptions:        ����FSMC
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ��8875д����
** input parameters:    cmd:Command
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ��8875д����
** input parameters:    dat:Data
** output parameters:   ��
** Returned value:      ��
***************************************************************************************/
static void RA8875_WriteData(unsigned short dat)
{
	LCD->LCD_RAM = dat;	
}

/**************************************************************************************
** Function name:       unsigned char RA8875_ReadStatus(void)
** Descriptions:        ��RA8875״̬
** input parameters:    ��
** output parameters:   ��
** Returned value:      temp:�Ĵ���״̬
***************************************************************************************/
static unsigned char RA8875_ReadStatus(void)
{
	unsigned short temp = 0;
	temp = LCD->LCD_REG;
	return (unsigned char)temp;
}

/**************************************************************************************
** Function name:       unsigned char RA8875_WaitBusy(void)
** Descriptions:        æ�ȴ�
** input parameters:    ��
** output parameters:   ��
** Returned value:      0:æ  1:��æ
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
** Descriptions:        ��ָ���Ĵ���д����
** input parameters:    reg:�Ĵ���  value:����
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:       ��ָ���Ĵ�������
** input parameters:   reg:�Ĵ���  value:����
** output parameters:  ���ݵ�ַ
** Returned value:     ��
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
** Descriptions:       ��ָ���Ĵ�������
** input parameters:   regadd:�Ĵ�����ַ
** output parameters:  ��
** Returned value:     ��ȡ�ļĴ�������
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
** Descriptions:       ������ʾ����
** input parameters:   x1:x����� x2:x���յ�  y1:y����� y2:y���յ�
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_OpenWindow(unsigned short x1,unsigned short x2,unsigned short y1,unsigned short y2)
{
	RA8875_WriteReg(0x30,(unsigned char)(x1));   //����ˮƽ���
	RA8875_WriteReg(0x31,(x1 >> 8));
	
	RA8875_WriteReg(0x34,(unsigned char)(x2));    //����ˮƽ�յ�
	RA8875_WriteReg(0x35,(x2 >> 8));


	RA8875_WriteReg(0x32,(unsigned char)(y1));    //���ô�ֱ���
	RA8875_WriteReg(0x33,(y1 >> 8));

	RA8875_WriteReg(0x36,(unsigned char)(y2));     //���ô�ֱ�յ�
	RA8875_WriteReg(0x37,(y2 >> 8));
}

/**************************************************************************************
** Function name:      void RA8875_Init(void)
** Descriptions:       RA8875��������ʼ��
** input parameters:   ��
** output parameters:  ��
** Returned value:     ��
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
** Descriptions:       ����Ļ����ָ������ɫ��������������� 0xffff
** input parameters:   Color      ���ֵ
** output parameters:  ��
** Returned value:     ��
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
** Descriptions:       ���ù��
** input parameters:   x,y����Ļ����
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void Set_TextCursor(uint32_t x,uint32_t y)   //���Թ��
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
** Descriptions:       ���ù��
** input parameters:   x,y����Ļ����
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void Set_GraphicsCursor(uint32_t x,uint32_t y)  //�ڴ���
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
** Descriptions:       ���(ָ����ɫ)
** input parameters:   x,y����Ļ����
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void LCD_DrawPoint1(uint32_t x,uint32_t y,uint32_t color)
{
	Set_GraphicsCursor(x,y);         //�����Դ���
	RA8875_WriteCmd(0x02);           //����д����
	RA8875_WriteData(color);         //��ʼд����
}

/**************************************************************************************
** Function name:      void LCD_DrawPoint(uint32_t x,uint32_t y)
** Descriptions:       ���(Ĭ����ɫ)
** input parameters:   x,y����Ļ����
** output parameters:  ��
** Returned value:     ��
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

   RA8875_WriteCmd(0x02);      //�����Σ���ֹ����
   Serial_Data=LCD->LCD_RAM;
   RA8875_WriteCmd(0x02);
   Serial_Data =  LCD->LCD_RAM;

//  	write_cmd(0x41);
//  	write_data(0x00);
  return Serial_Data;

}


/**************************************************************************************
** Function name:      void RA8875_DispAscii(uint16_t x, uint16_t y, char *_ptr)
** Descriptions:       ��ʾASCII�ַ�
** input parameters:   x,y����Ļ���� _prt �ַ����׵�ַ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_DispAscii(uint16_t x, uint16_t y, char *_ptr)
{
	Set_TextCursor(x, y);             //������ʾ���

	RA8875_WriteReg(0x40, (1 << 7));	// ����Ϊ�ı�ģʽ 

  RA8875_WriteReg(0x20, (1 << 3));	// ˮƽɨ�跽���趨
	RA8875_WriteReg(0x20, (1 << 2));	    // ��ֱɨ�跽���趨
	
	//RA8875_WriteReg(0x22, (0 << 6));	    // ����͸��
  
	/* ѡ��CGROM font; ѡ���ڲ�CGROM; �ڲ�CGROM ����ѡ��ISO/IEC 8859-1. */
	RA8875_WriteReg(0x2F, 0x00);
	RA8875_WriteReg(0x21, (0 << 7) | (0 << 5) | (0 << 1) | (0 << 0));

  //RA8875_WriteReg(0x22, (1<<4));   //������ת90��
	 
   //   RA8875_WriteReg(0x22, (1 << 3) | (1 << 2) );   //ˮƽ�Ŵ�4��
 	 
   //	  RA8875_WriteReg(0x22, (1 << 1) | (1 << 0) );   //��ֱ�Ŵ�4��



	 RA8875_WriteCmd(0x02); 		//�����趨RA8875 �����ڴ�(DDRAM��CGRAM)��ȡ/д��ģʽ 

	while (*_ptr != 0)
	{
	  RA8875_WriteData(*_ptr);
		while ((RA8875_ReadStatus() & 0x80) == 0x80);	// ����ȴ��ڲ�д��������� 
		_ptr++;
	}

	RA8875_WriteReg(0x40, (0 << 7));	// ��ԭΪͼ��ģʽ 
}

/**************************************************************************************
** Function name:      void RA8875_SetFrontColor(uint16_t Color)
** Descriptions:       �趨ǰ��ɫ
** input parameters:   Color:��ɫ
** output parameters:  ��
** Returned value:     ��
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
** Descriptions:       �趨ǰ��ɫ
** input parameters:   Color:��ɫ
** output parameters:  ��
** Returned value:     ��
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
** Descriptions:       RA8875 ͼ��1��ʾ��
** input parameters:   ��
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_Layer1Visable(void)
{

	RA8875_WriteReg(0x52, RA8875_readreg(0x52) & 0xF8);	//ֻ��ͼ��1 ��ʾ 
}


/**************************************************************************************
** Function name:      void RA8875_Layer2Visable(void)
** Descriptions:       ֻ��ʾͼ��2
** input parameters:   ��
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_Layer2Visable(void)
{

	RA8875_WriteReg(0x52, (RA8875_readreg(0x52) & 0xF8) | 0x01);	//ֻ��ͼ��2 ��ʾ 
}


/**************************************************************************************
** Function name:      void  RA8875_DrawLine(uint16_t X1 , uint16_t Y1 , uint16_t X2 , uint16_t Y2 , uint16_t _usColor)
** Descriptions:       ����RA8875��Ӳ����ͼ���ܣ���2��仭һ��ֱ�ߡ�
** input parameters:   X1, Y1 ����ʼ��X����
**			               X2, Y2 ����ֹ��Y����
**                     Color  : ��ɫ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_DrawLine(uint16_t X1 , uint16_t Y1 , uint16_t X2 , uint16_t Y2 , uint16_t Color)
{

	
	RA8875_WriteReg(0x91, X1);               //����������� 
	RA8875_WriteReg(0x92, X1 >> 8);
	RA8875_WriteReg(0x93, Y1);
	RA8875_WriteReg(0x94, Y1 >> 8);

	
	RA8875_WriteReg(0x95, X2);               //�����յ����� 
	RA8875_WriteReg(0x96, X2 >> 8);
	RA8875_WriteReg(0x97, Y2);
	RA8875_WriteReg(0x98, Y2 >> 8);

	RA8875_SetFrontColor(Color);	            //����ֱ�ߵ���ɫ 

  if (RA8875_WaitBusy() == 0)
	{
		RA8875_WriteReg(0x90, (1 << 7) | (0 << 4) | (0 << 0));	//��ʼ��ֱ�� 
//	while (RA8875_ReadReg(0x90) & (1 << 7));				         //�ȴ����� 
	
  }
	
}

/**************************************************************************************
** Function name:      void RA8875_DrawRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
** Descriptions:       RA8875Ӳ�����ܻ��ƾ���
** input parameters:   X,Y   ���������Ͻǵ�����
**		                 Height�����εĸ߶�
**		                 Width �����εĿ��
**                     Color : ��ɫ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_DrawRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
{
	
	RA8875_WriteReg(0x91, X);                   // ����������� 
	RA8875_WriteReg(0x92, X >> 8);
	RA8875_WriteReg(0x93, Y);
	RA8875_WriteReg(0x94, Y >> 8);

	
	RA8875_WriteReg(0x95,  X + Width - 1);        // �����յ����� 
	RA8875_WriteReg(0x96, (X + Width - 1) >> 8);
	RA8875_WriteReg(0x97,  Y + Height - 1);
	RA8875_WriteReg(0x98, (Y + Height - 1) >> 8);

	RA8875_SetFrontColor(Color);	                  //������ɫ 

  if(RA8875_WaitBusy() == 0)
	{
	  RA8875_WriteReg(0x90, (1 << 7) | (0 << 5) | (1 << 4) | (0 << 0));	 //��ʼ������ 
//	while (RA8875_ReadReg(0x90) & (1 << 7));							             // �ȴ����� 
	} 
}

/**************************************************************************************
** Function name:      void RA8875_FillRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
** Descriptions:       RA8875Ӳ�����ܻ��ƾ���
** input parameters:   X,Y   ���������Ͻǵ�����
**		                 Height�����εĸ߶�
**		                 Width �����εĿ��
**                     Color : �����ɫ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_FillRect(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t Color)
{
	
	RA8875_WriteReg(0x91, X);                  // ����������� 
	RA8875_WriteReg(0x92, X >> 8);
	RA8875_WriteReg(0x93, Y);
	RA8875_WriteReg(0x94, Y >> 8);

	
	RA8875_WriteReg(0x95, X + Width - 1);          // �����յ����� 
	RA8875_WriteReg(0x96, (X + Width - 1) >> 8);
	RA8875_WriteReg(0x97, Y + Height - 1);
	RA8875_WriteReg(0x98, (Y + Height - 1) >> 8);

	RA8875_SetFrontColor(Color);	                   //������ɫ 

	RA8875_WriteReg(0x90, (1 << 7) | (1 << 5) | (1 << 4) | (0 << 0));	// ��ʼ������ 
//	while (RA8875_ReadReg(0x90) & (1 << 7));							          // �ȴ����� 

}

/**************************************************************************************
** Function name:      void RA8875_DrawCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
** Descriptions:       RA8875Ӳ�����ܻ���Բ��
** input parameters:   X,Y   ��Բ������
**		                 Radius���뾶
**                     Color : ͼ����ɫ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_DrawCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
{
	/*
		RA8875 ֧��Բ�λ�ͼ���ܣ���ʹ�����Լ��׻���ٵ�MCU �Ϳ�����TFT ģ���ϻ�Բ�����趨
		Բ�����ĵ�REG[99h~9Ch]��Բ�İ뾶REG[9Dh]��Բ����ɫREG[63h~65h]��Ȼ��������ͼ
		REG[90h] Bit6 = 1��RA8875 �ͻὫԲ��ͼ��д��DDRAM����Ե���TFT ģ���ϾͿ�����ʾ��
		����Բ�����趨REG[90h] Bit5 = 1����ɻ���һʵ��Բ (Fill)�����趨REG[90h] Bit5 = 0�����
		��������Բ (Not Fill
	*/
	
	if (Radius > 255)               //����Բ�İ뾶 
	{
		return;
	}

	
	RA8875_WriteReg(0x99, X);        //����Բ������ 
	RA8875_WriteReg(0x9A, X >> 8);
	RA8875_WriteReg(0x9B, Y);
	RA8875_WriteReg(0x9C, Y >> 8);

	RA8875_WriteReg(0x9D, Radius);	//����Բ�İ뾶 

	RA8875_SetFrontColor(Color);	  // ������ɫ

	RA8875_WriteReg(0x90, (1 << 6) | (0 << 5));				//��ʼ��Բ, ����� 
//	while (RA8875_ReadReg(0x90) & (1 << 6));				//�ȴ����� 

}


/**************************************************************************************
** Function name:      void RA8875_FillCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
** Descriptions:       RA8875Ӳ�����ܻ���Բ��
** input parameters:   X,Y   ��Բ������
**		                 Radius���뾶
**                     Color : �����ɫ
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_FillCircle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Color)
{
	if (Radius > 255)                     //����Բ�İ뾶 
	{
		return;
	}

	
	RA8875_WriteReg(0x99, X);              // ����Բ������ 
	RA8875_WriteReg(0x9A, X >> 8);
	RA8875_WriteReg(0x9B, Y);
	RA8875_WriteReg(0x9C, Y >> 8);

	RA8875_WriteReg(0x9D, Radius);	                  // ����Բ�İ뾶 

	RA8875_SetFrontColor(Color);	                   //  ������ɫ 

	RA8875_WriteReg(0x90, (1 << 6) | (1 << 5));				// ��ʼ��Բ, ��� 
//	while (RA8875_ReadReg(0x90) & (1 << 6));				//�ȴ����� 

}

/**************************************************************************************
** Function name:      void RA8875_SetDispWin(uint16_t X, uint16_t Y, uint16_t Height, uint16_t _usWidth)
** Descriptions:       ������ʾ����
** input parameters:   X,Y   ����ʾ����		                
**                     Height: ���ڸ߶�
**		                 Width : ���ڿ��
** output parameters:  ��
** Returned value:     ��
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
** Descriptions:       �˳�������ʾģʽ����Ϊȫ����ʾģʽ
** input parameters:   ��               
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_QuitWinMode(void)
{
	RA8875_SetDispWin(0, 0, 480, 800);
}

/**************************************************************************************
** Function name:      void RA8875_DrawBMP(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t *_ptr)
** Descriptions:       ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
** input parameters:   X, Y    : ͼƬ������
**		                 Height  ��ͼƬ�߶�
**		                 Width   ��ͼƬ���
**		                 _ptr    ��ͼƬ����ָ��              
** output parameters:  ��
** Returned value:     ��
***************************************************************************************/
void RA8875_DrawBMP(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Width, uint16_t *_ptr)
{
	uint32_t index = 0;
	const uint16_t *p;
	RA8875_SetDispWin(X, Y, Height, Width);//����ͼƬ��λ�úʹ�С�� ��������ʾ����

  RA8875_WriteReg(0x40, (0 << 7));	      // ����Ϊ��ͼģʽ 

	RA8875_WriteCmd(0x02); 		              // ׼����д�Դ� 

	p = _ptr;
	for (index = 0; index < Height * Width; index++)
	{
		
		RA8875_WriteData(*p++);
	}

	RA8875_QuitWinMode();                     // �˳����ڻ�ͼģʽ 
}
