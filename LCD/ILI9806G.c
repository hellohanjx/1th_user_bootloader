/*
5������ili9806g����оƬ
*/



#include "lcd_header.h" 


#if SCREEN == SCREENILI9806
 
/*
�����Ĵ�����ַ
*/
#define Bank1_LCD_D    ((uint32_t)0x6C000002)    //Disp Data ADDR
#define Bank1_LCD_C    ((uint32_t)0x6C000000)	  //Disp Reg ADDR


/*
???????
*/
static volatile uint16_t  X_SIZE =	854;
static volatile uint16_t  Y_SIZE =	480;


#define Lcd_Light_ON   GPIOG->BSRR = GPIO_Pin_14;
#define Lcd_Light_OFF  GPIOG->BRR  = GPIO_Pin_14;


 /*
 * ��������LCD_GPIO_Config
 * ����  ������FSMC����LCD��I/O
 * ����  ����
 * ���  ����
 * ����  ���ڲ�����        
 */

static void LCD_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
	/* config lcd gpio clock base on FSMC */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
						 RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | 
						 RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | 
						 RCC_APB2Periph_GPIOG, ENABLE);

    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    /* config tft Reset gpio */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    /* config tft BL gpio ���⿪�ؿ�������*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ; 	 
    GPIO_Init(GPIOG, &GPIO_InitStructure);  		   
    
    /* config tft data lines base on FSMC
	 * data lines,FSMC-D0~D15: PD 14 15 0 1,PE 7 8 9 10 11 12 13 14 15,PD 8 9 10
	 */	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure); 
    
    /* config tft control lines base on FSMC
	 * PD4-FSMC_NOE  :LCD-RD
   * PD5-FSMC_NWE  :LCD-WR
	 * PG12-FSMC_NE4  :LCD-CS
   * PF0-FSMC_A0  :LCD-RS
	 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
    GPIO_Init(GPIOG, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ; 
    GPIO_Init(GPIOF, &GPIO_InitStructure);  
    
    /* tft control gpio init */	 
    //GPIO_SetBits(GPIOD, GPIO_Pin_13);		 // Reset				
    GPIO_SetBits(GPIOD, GPIO_Pin_4);		 // RD = 1  
    GPIO_SetBits(GPIOD, GPIO_Pin_5);		 // WR = 1 
		GPIO_SetBits(GPIOF, GPIO_Pin_0);		 // RS = 1
    GPIO_SetBits(GPIOG, GPIO_Pin_12);		 //	CS = 1
		GPIO_SetBits(GPIOG, GPIO_Pin_14);	 	 //	BL = 1 
		
		
}

/*
 * ��������LCD_FSMC_Config
 * ����  ��LCD  FSMC ģʽ����
 * ����  ����
 * ���  ����
 * ����  ���ڲ�����        
 */

static void LCD_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p; 
    
    
    p.FSMC_AddressSetupTime = 0x01;	 //��ַ����ʱ��
    p.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ��
    p.FSMC_DataSetupTime = 0x02;		 //���ݽ���ʱ��
    p.FSMC_BusTurnAroundDuration = 0x00;
    p.FSMC_CLKDivision = 0x00;
    p.FSMC_DataLatency = 0x00;

    p.FSMC_AccessMode = FSMC_AccessMode_B;	 // һ��ʹ��ģʽB������LCD
    
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p; 
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
    
    /* Enable FSMC Bank_SRAM Bank */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  
}

static void LCD_delay(char j)
{
	volatile uint16_t i;	
	while(j--)
	for(i=7200;i>0;i--);
}

/*
д����
*/
static void WriteComm(uint16_t CMD)
{			
	*(volatile uint16_t *) (Bank1_LCD_C) = CMD;
}

/*
д����
*/
static void WriteData(uint16_t tem_data)
{			
	*(volatile uint16_t *) (Bank1_LCD_D) = tem_data;
}

/*
��Ļ��λ
*/
static void bsp_lcd_rst(void)
{
	delay_ms(300);
	GPIO_ResetBits(GPIOG,GPIO_Pin_13);
	delay_ms(100);
	GPIO_SetBits(GPIOG,GPIO_Pin_13);
	delay_ms(300);
}


/*
���ú�����
������0������1����
*/
void bsp_lcd_set_directrion(uint8_t type, uint16_t *x, uint16_t *y)
{
	if(type == 0)
	{
		WriteComm(0x36); 
		WriteData(0x60);
		X_SIZE	= 854;
		Y_SIZE	= 480;
	}
	else
	{//����
		WriteComm(0x36); 
		WriteData(0x00);
		X_SIZE	= 480;
		Y_SIZE	= 854;
	}
	*x = X_SIZE;
	*y = Y_SIZE;

}


/**********************************************
Lcd��ʼ������
***********************************************/
void bsp_lcd_hard_congfigure(void)
{	
	LCD_GPIO_Config();
	LCD_FSMC_Config();
	bsp_lcd_rst();	//bootloader �Ѿ���λ�������ﲻ��λ
		
	//************* Start Initial Sequence **********//
	WriteComm(0xFF); // EXTC Command Set enable register 
	WriteData(0xFF); 
	WriteData(0x98); 
	WriteData(0x06); 

	WriteComm(0xBA); // SPI Interface Setting 
	WriteData(0xE0); 

	WriteComm(0xBC); // GIP 1 
	WriteData(0x03); 
	WriteData(0x0F); 
	WriteData(0x63); 
	WriteData(0x69); 
	WriteData(0x01); 
	WriteData(0x01); 
	WriteData(0x1B); 
	WriteData(0x11); 
	WriteData(0x70); 
	WriteData(0x73); 
	WriteData(0xFF); 
	WriteData(0xFF); 
	WriteData(0x08); 
	WriteData(0x09); 
	WriteData(0x05); 
	WriteData(0x00);
	WriteData(0xEE); 
	WriteData(0xE2); 
	WriteData(0x01); 
	WriteData(0x00);
	WriteData(0xC1); 

	WriteComm(0xBD); // GIP 2 
	WriteData(0x01); 
	WriteData(0x23); 
	WriteData(0x45); 
	WriteData(0x67); 
	WriteData(0x01); 
	WriteData(0x23); 
	WriteData(0x45); 
	WriteData(0x67); 

	WriteComm(0xBE); // GIP 3 
	WriteData(0x00); 
	WriteData(0x22); 
	WriteData(0x27); 
	WriteData(0x6A); 
	WriteData(0xBC); 
	WriteData(0xD8); 
	WriteData(0x92); 
	WriteData(0x22); 
	WriteData(0x22); 

	WriteComm(0xC7); // Vcom 
	WriteData(0x1E);
	 
	WriteComm(0xED); // EN_volt_reg 
	WriteData(0x7F); 
	WriteData(0x0F); 
	WriteData(0x00); 

	WriteComm(0xC0); // Power Control 1
	WriteData(0xE3); 
	WriteData(0x0B); 
	WriteData(0x00);
	 
	WriteComm(0xFC);
	WriteData(0x08); 

	WriteComm(0xDF); // Engineering Setting 
	WriteData(0x00); 
	WriteData(0x00); 
	WriteData(0x00); 
	WriteData(0x00); 
	WriteData(0x00); 
	WriteData(0x02); 

	WriteComm(0xF3); // DVDD Voltage Setting 
	WriteData(0x74); 

	WriteComm(0xB4); // Display Inversion Control 
	WriteData(0x00); 
	WriteData(0x00); 
	WriteData(0x00); 

	WriteComm(0xF7); // 480x854
	WriteData(0x81); 

	WriteComm(0xB1); // Frame Rate 
	WriteData(0x00); 
	WriteData(0x10); 
	WriteData(0x14); 

	WriteComm(0xF1); // Panel Timing Control 
	WriteData(0x29); 
	WriteData(0x8A); 
	WriteData(0x07); 

	WriteComm(0xF2); //Panel Timing Control 
	WriteData(0x40); 
	WriteData(0xD2); 
	WriteData(0x50); 
	WriteData(0x28); 

	WriteComm(0xC1); // Power Control 2 
	WriteData(0x17);
	WriteData(0X85); 
	WriteData(0x85); 
	WriteData(0x20); 

	WriteComm(0xE0); 
	WriteData(0x00); //P1 
	WriteData(0x0C); //P2 
	WriteData(0x15); //P3 
	WriteData(0x0D); //P4 
	WriteData(0x0F); //P5 
	WriteData(0x0C); //P6 
	WriteData(0x07); //P7 
	WriteData(0x05); //P8 
	WriteData(0x07); //P9 
	WriteData(0x0B); //P10 
	WriteData(0x10); //P11 
	WriteData(0x10); //P12 
	WriteData(0x0D); //P13 
	WriteData(0x17); //P14 
	WriteData(0x0F); //P15 
	WriteData(0x00); //P16 

	WriteComm(0xE1); 
	WriteData(0x00); //P1 
	WriteData(0x0D); //P2 
	WriteData(0x15); //P3 
	WriteData(0x0E); //P4 
	WriteData(0x10); //P5 
	WriteData(0x0D); //P6 
	WriteData(0x08); //P7 
	WriteData(0x06); //P8 
	WriteData(0x07); //P9 
	WriteData(0x0C); //P10 
	WriteData(0x11); //P11 
	WriteData(0x11); //P12 
	WriteData(0x0E); //P13 
	WriteData(0x17); //P14 
	WriteData(0x0F); //P15 
	WriteData(0x00); //P16

	WriteComm(0x35); //Tearing Effect ON 
	WriteData(0x00); 

	WriteComm(0x3A); 
	WriteData(0x55); 
	WriteComm(0x11); //Exit Sleep 
	LCD_delay(10);
	WriteComm(0x29); // Display On 
	LCD_delay(10);

//	bsp_lcd_set_directrion(0);//������Ļ����
	//Lcd_Light_ON;`
	GPIO_SetBits(GPIOG, GPIO_Pin_14);	 	 //	BL = 1 
	
}


/******************************************
��������Lcdд�����
���ܣ���Lcdָ��λ��д��Ӧ�����������
��ڲ�����Index ҪѰַ�ļĴ�����ַ
          ConfigTemp д������ݻ�����ֵ
******************************************/
static void LCD_WR_REG(uint16_t Index,uint16_t CongfigTemp)
{
	*(__IO uint16_t *) (Bank1_LCD_C) = Index;	
	*(__IO uint16_t *) (Bank1_LCD_D) = CongfigTemp;
}


/*
ѡ����������
������Xstart��Ystart����ʼ����
		Xend��Yend����������
����Ҫע�⣺��8875�У�Xend��Yend ��ָ��ʾ���ȣ�������һ������
*/
void bsp_lcd_block_select(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	Xend = Xstart + Xend - 1;
	Yend = Ystart + Yend - 1;
	
	WriteComm(0x2a);   
	WriteData(Xstart>>8);
	WriteData(Xstart&0xff);
	WriteData(Xend>>8);
	WriteData(Xend&0xff);

	WriteComm(0x2b);   
	WriteData(Ystart>>8);
	WriteData(Ystart&0xff);
	WriteData(Yend>>8);
	WriteData(Yend&0xff);
	
	WriteComm(0x2c);
}

/*
����˼��ȡ�õ�ǰ�����Ϣ
*/
 uint16_t bsp_lcd_get_pixel( uint16_t x, uint16_t y)
{
	WriteComm(0x2a);   
	WriteData(x>>8);
	WriteData(x);
	WriteData(x>>8);
	WriteData(x);

	WriteComm(0x2b);   
	WriteData(y>>8);
	WriteData(y);
	WriteData(y>>8);
	WriteData(y);

	WriteComm(0x2e);
	
	x = *(__IO uint16_t *) (Bank1_LCD_D);x=1;while(--x);
	x = *(__IO uint16_t *) (Bank1_LCD_D);
	y = *(__IO uint16_t *) (Bank1_LCD_D);
// 	printf("RIN=%04x\r\n",b);

	return (x&0xf800)|((x&0x00fc)<<3)|(y>>11);
}



/*
��ָ�������ϴ�һ����
������x�������ꣻy�����ꣻcolor����ɫ
*/
void DrawPixel(uint16_t x, uint16_t y, int Color)
{
	bsp_lcd_block_select(x,x,y,y);
	*(__IO uint16_t *) (Bank1_LCD_D) = Color;
}




volatile uint16_t *bsp_get_reg(void)
{
	return (__IO uint16_t *) (Bank1_LCD_C);
}
volatile uint16_t *bsp_get_ram(void)
{
	return (__IO uint16_t *) (Bank1_LCD_D);
}
uint16_t bsp_get_xsize(void)
{
	return X_SIZE;
}
uint16_t bsp_get_ysize(void)
{
	return Y_SIZE;
}


/*
���ָ������
*/
void lcd_fill_color(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
	uint32_t index = 0, num = yLong * xLong;
	
	bsp_lcd_block_select(xStart, xLong, yStart, yLong );//????

//	RA8875Busy = 1;

	for (index = 0; index < num; index++)
	{
		*(__IO uint16_t *) (Bank1_LCD_D) = (Color);
	}
//	RA8875Busy = 0;
}

/*
���ܣ�����Ļ�ϴ�ӡһ������ͼ����Ҫ������ӡ���֡�
����: lattice ��Ҫ��ӡ�ĵ���xSize ���򳤶ȣ�ySize ���򳤶ȣ�
x��ʼ�����꣬y��ʼ�����꣬forcground ǰ�� background ����
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
			*(__IO uint16_t *) (Bank1_LCD_D) = forcground;
		else
			*(__IO uint16_t *) (Bank1_LCD_D) = background;
	}
//	RA8875Busy = 0;
}

/*
��ʾ��ά��
������x��y��ʼ����
xylong����ʾ��ȣ���Ϊ��ά���Ƿ��ε�
scal���Ŵ�ϵ������ά����԰�����������Ŵ�
*/
void show_ercode(uint16_t x, uint16_t y, uint16_t xylong, uint8_t scale , uint8_t dat[][41]) 
{
	uint32_t i, j ,num;

	num = xylong*scale;//���٣����治��ÿ�δ��ǰ����
	bsp_lcd_block_select(x, (num), y , (num) );
	
	for(i = 0; i < num; i++)
	{
		for(j = 0; j < num; j++)
		{
			*(__IO uint16_t *) (Bank1_LCD_D) = dat[i/scale][j/scale]? BLUE:WHITE;
		}
			 
	}
}



#endif
