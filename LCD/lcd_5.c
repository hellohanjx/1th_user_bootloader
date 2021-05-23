#include "stm32f10x_conf.h"

/*
 * 函数名：LCD_GPIO_Config
 * 描述  ：根据FSMC配置LCD的I/O
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用        
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
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		
    //GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* config tft BL gpio 背光开关控制引脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ; 	 
    GPIO_Init(GPIOD, &GPIO_InitStructure);  		   
    
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
    
	
	//bl
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	//rst
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	
	
    /* tft control gpio init */	 
    //GPIO_SetBits(GPIOD, GPIO_Pin_13);		 // Reset				
    GPIO_SetBits(GPIOD, GPIO_Pin_4);		 // RD = 1  
    GPIO_SetBits(GPIOD, GPIO_Pin_5);		 // WR = 1 
	GPIO_SetBits(GPIOF, GPIO_Pin_0);		 // RS = 1
    GPIO_SetBits(GPIOG, GPIO_Pin_12);		 //	CS = 1
	GPIO_SetBits(GPIOD, GPIO_Pin_3);	 	 //	BL = 1 
		
		
}

/*
 * 函数名：LCD_FSMC_Config
 * 描述  ：LCD  FSMC 模式配置
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用        
 */

static void LCD_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p; 
    
    
    p.FSMC_AddressSetupTime = 0x01;	 //地址建立时间
    p.FSMC_AddressHoldTime = 0x00;	 //地址保持时间
    p.FSMC_DataSetupTime = 0x02;		 //数据建立时间
    p.FSMC_BusTurnAroundDuration = 0x00;
    p.FSMC_CLKDivision = 0x00;
    p.FSMC_DataLatency = 0x00;

    p.FSMC_AccessMode = FSMC_AccessMode_B;	 // 一般使用模式B来控制LCD
    
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

volatile void LCD_delay(char j)
{
	volatile uint16_t i;	
		while(j--)
	for(i=7200;i>0;i--);
}
static void LCD_Rst(void)
{			
    GPIO_ResetBits(GPIOG, GPIO_Pin_13);
    LCD_delay(50);//这里时间改长些，看看能不能接绝闪屏的问题（那各升压芯片发烫，可能是瞬间电流过大）	   
    GPIO_SetBits(GPIOG, GPIO_Pin_13 );		 	 
    LCD_delay(50);	
}

 void WriteComm(uint16_t CMD)
{			
	*(__IO uint16_t *) (Bank1_LCD_C) = CMD;
}
 void WriteData(uint16_t tem_data)
{			
	*(__IO uint16_t *) (Bank1_LCD_D) = tem_data;
}


/**********************************************
Lcd初始化函数
***********************************************/
void Lcd_Initialize(void)
{	
	LCD_GPIO_Config();
	LCD_FSMC_Config();
	LCD_Rst();
	
	
		
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

	WriteComm(0x36); 
	WriteData(0x60); 

	WriteComm(0x3A); 
	WriteData(0x55); 

	WriteComm(0x11); //Exit Sleep 
	LCD_delay(120); 
	WriteComm(0x29); // Display On 
	LCD_delay(10);
	Lcd_Light_ON;
	
	//WriteComm(0x3A); WriteData(0x55);
	//WriteComm(0x36); WriteData(0xA8);
	//Lcd_ColorBox(0,0,854,480,BLACK);
	//Lcd_ColorBox(50,80,300,200,RED);
	//LCD_ShowString("HAB",200 ,150,WHITE,FONT_64);
	//LCD_ShowString("兴百",400 ,150,WHITE,FONT_96);
	
	//LCD_ShowString("abcdefghijklmnopqrstuvwxyz+-*/=-_#\\=|<>?[] .好请插入盘等待升级中稍候完成",200 ,300,WHITE,FONT_48);

	//LCD_ShowNum(0xffffffff,200 ,400,WHITE,FONT_48);

// 	LCD_Fill_Pic(80,160,320,480, gImage_MM_T035);
// 	BlockWrite(0,0,799,479);
}
/******************************************
函数名：Lcd写命令函数
功能：向Lcd指定位置写入应有命令或数据
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_WR_REG(uint16_t Index,uint16_t CongfigTemp)
{
	*(__IO uint16_t *) (Bank1_LCD_C) = Index;	
	*(__IO uint16_t *) (Bank1_LCD_D) = CongfigTemp;
}

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
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
 uint16_t GetPoint( uint16_t x, uint16_t y)
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
/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void Lcd_ColorBox(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
	uint32_t temp;

	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	for (temp=0; temp<xLong*yLong; temp++)
	{
		*(__IO uint16_t *) (Bank1_LCD_D) = Color;
	}
}

/******************************************
函数名：Lcd图像填充100*100
功能：向Lcd指定位置填充图像
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_Fill_Pic(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const unsigned char* pic)
{
  unsigned long i;
	unsigned int j;

// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0x00); //横屏，从左下角开始，从左到右，从下到上
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	for (i = 0; i < pic_H*pic_V*2; i+=2)
	{
		j=pic[i];
		j=j<<8;
		j=j+pic[i+1];
		*(__IO uint16_t *) (Bank1_LCD_D) = j;
	}
// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0xA0);
}

//在指定座标上打一个点
void DrawPixel(uint16_t x, uint16_t y, int Color)
{
	BlockWrite(x,x,y,y);

	*(__IO uint16_t *) (Bank1_LCD_D) = Color;
}

//====================================
//字体显示
//list 字表
//listLen 一个字符的字节数
//locate 字符在字表的起始位置
//x,y 坐标
//row 行字节数 行数 = row * 8
//line 列数
//color 颜色
//====================================
void LCD_ShowChar(const unsigned char *list, uint32_t listLen, uint16_t locate, 
									uint16_t x, uint16_t y, uint32_t row, uint8_t line, uint16_t color)
{  
	uint8_t temp;
	uint32_t rowByte;
	uint32_t pos,t,num;
	uint32_t x0 = x;	

	num = 0;
	//描绘每列
	for(pos = 0; pos < line; pos++)
	{
		//描绘每行
		for(rowByte = 0; rowByte < row; rowByte++)
		{
			temp = list[locate*listLen + num];		//调用字体
			num++;
			//描绘1个字节的点
			for(t = 0; t < 8; t++)
			{                 
				if(temp & 0x01)
				{
					DrawPixel(x,y,color);
				}
				else //不要背景色
					DrawPixel(x,y,BLACK);
				temp >>= 1; 
				x++;
			}
		}
		x = x0;
		y++;
	}
}   
//===========================
//查字母表
//key 字符
//letter 字表
//mode 汉字/英文
//===========================
uint16_t FindLetter(const char *key,const char *letter, uint8_t mode)
{
	uint16_t i=0;
	
	if(mode == LCD_EN)
	{
		while(letter[i] != '\0')
		{
			if(letter[i] == *key)
			return i;
			i++;
		}
	}
	else if(mode == LCD_HZ)
	{
		while(letter[i] != '\0')
		{
			if((letter[i] == key[0]) && (letter[i+1] == key[1]))// && (letter[i+2] == key[2]))
			return (i/2);//字符在字表中的相对位置
			i += 2;
		}
	}
	return 0xffff;
}
//===========================
//显示字符串
//zi 字符
//x y 坐标
//color 颜色
//fontSize 字体
//注意：只支持16的倍数的字体
//===========================
void LCD_ShowString(const char * zi,uint16_t x, uint16_t y ,uint16_t color,uint8_t fontSize)
{
	uint32_t i;
	uint32_t locate;							//字符在字表中的相对位置
	uint32_t fontoffset;					//每个字符水平偏移位数
	uint32_t listLen;						//表中每个字符占用的字节数
	uint32_t lineByte;						//每行的显示字节数

	const char *letter;
	const unsigned char *list;
	i = 0;
	
	//字表
	switch(fontSize)
	{
		case FONT_64:
			letter = LT_64;	//内码表
			list = asc2_64;	//点阵表
		break;
		case FONT_96:
			letter = LT_96;
			list = asc2_96;
		break;
		case FONT_48:
			letter = LT_48;
			list = asc2_48;	
		break;
	}
	
	while(zi[i]!='\0')
	{       
		if(x > 854)//自动换行
		{
			x = 0;
			y += fontSize;
		}
		if(y > 480)//回到0,0位置
		{
			y = x = 0;
		}
		
		if(zi[i] >= 0x81 && zi[i] <= 0xFE && zi[i+1] != 0xFF)//gbk内码
		{
			fontoffset = fontSize;
			listLen = fontSize*fontSize/8;
			lineByte = fontSize/8;
			locate = FindLetter(&zi[i], letter, LCD_HZ);//寻找字符在字母表中位置
			i += 2;
		}
		else//字母
		{
			fontoffset = fontSize/2;
			listLen = fontSize*fontSize/16;	
			lineByte = fontSize/16;
			locate = FindLetter(&zi[i], letter, LCD_EN);
			i++;
		}
		if(locate != 0xffff)
		LCD_ShowChar(list, listLen, locate, x, y, lineByte, fontSize, color);
		x += fontoffset;//这个根据字体不同移位不同
	}
}
//===========================
//显示数字 最大0xffffffff
//value 显示值
//num 位数
//type 0：变长显示 1定长显示
//x y 坐标
//color 颜色
//size 字体
//===========================
void LCD_ShowNum(uint32_t value, uint8_t num, uint8_t type, uint16_t x, uint16_t y, uint16_t color, uint8_t fontSize)
{
	char tmp[11]="          ";
	uint8_t i;
	for(i=0;i<num && i<10;i++)
	{
		tmp[9-i] = (value%10)+48;
		value = value/10;
		if(value == 0 && type == 0)
			break;
	}
	LCD_ShowString(&tmp[10 - num], x,  y , color, fontSize);

}
//===========================
//清理屏幕
//===========================

