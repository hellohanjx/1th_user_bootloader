/*
lcd bsp支持包
*/

#include "lcd_drv.h" 
#include "lcd_header.h"  
#include "font_header.h"

/*
屏幕信息全局变量
*/
 volatile SCREEN_INFO g_lcd;  

/**********
对外接口函数
***********/

/*
设置打印字符串时的背景颜色
*/
void lcd_set_background(uint16_t background)
{
	g_lcd.background = background;
}

/*
设置打印字符串时的前景颜色
*/
void lcd_set_forceground(uint16_t forcegound)
{
	g_lcd.foreground = forcegound;
}

uint16_t lcd_get_xsize(void)
{
	return g_lcd.xSize;
}

uint16_t lcd_get_ysize(void)
{
	return g_lcd.ySize;
}






/*
lcd 参数配置
显示屏驱动应当是不关心颜色与字体的,分辨率是固定的
*/
static void lcd_set_par(void)
{
	g_lcd.background = BLACK;				//设置背景色
	g_lcd.foreground = YELLOW;				//设置前景色
	g_lcd.hightLightBackground = RED;		//设置高亮背景色
	g_lcd.hightLightforeground = BLACK;		//设置高亮前景色
	g_lcd.promptColor = YELLOW;				//设置提示信息颜色
	g_lcd.waringColor = RED;				//设置警告信息颜色
	g_lcd.xSize = bsp_get_xsize();
	g_lcd.ySize = bsp_get_ysize();
}




//===========================
//查字母表（就是查索引）
//参数：
//key 字符
//font 字体
//===========================
static const uint8_t* findLetter(const char *key,const BX_FONT font)
{
	uint16_t i=0;
	const uint8_t* lattice; //需要找到的点阵指针。
	const char* index;
	index = font.index;
	lattice = font.lattice;
	do
	{
		if(key[0] & 0x80)//如果搜索的是双字节
		{
			if(index[i] & 0x80)//如果索引当前也是双字节
			{
				if(key[0]==index[i] && key[1]==index[i+1])//如果匹配成功
				{
					return lattice;//返回点阵
				}
				else//匹配不成功
				{
					lattice+=font.size*font.size/8;	//2字节表示的字符占用的字节数
					i+=2;//跳过该索引
				}
			}
			else //如果索引是单字节
			{
				//跳过该索引
				lattice+=font.size*font.size/16;
				i++;
			}
		}
		else//如果搜索的是单字节
		{
			if(index[i] & 0x80)
			{
				//如果当前索引是双字节则跳过该索引搜索下一个
				lattice+=font.size*font.size/8;			
				i+=2;//跳过该索引
			}
			else
			{
				if(key[0]==index[i])//如果匹配成功
				{
					return lattice;//返回点阵
				}
				else
					lattice+=font.size*font.size/16;	//1字节表示的字符占用的字节数
				i++;
			}
		}
	}while(index[i]!='\0');
	
	return FALSE;	//找到返回地址，找不到返回0
	
}



/*
提供全功能字符串打印设置
入参：str 需要打印的字符串，x 起始行   y 起始列， font 字体，foreground 前景色（字体颜色），background 背景色（字体背景）
出参：无
*/
void  lcd_show_string(const char* str,uint16_t x ,uint16_t y, uint16_t foreground, uint16_t background, const BX_FONT font)
{
	uint8_t i;
	uint8_t width;
	const uint8_t *lattice;//点阵
	i=0;
	while(str[i]!='\0')
	{
		if(str[i] & 0x80 ) 	//汉字长宽相同(gbk 码中ASCII码 <= 127(0x7f))
			width=font.size;
		else				//如果是英文和数字，只占用一半宽度
			width=font.size/2;
		
		if(x+width>g_lcd.xSize)//右越界，移到下一行显示。
		{
//			y+=font.size;
//			x=0;
			return;
		}
		if(y+font.size>g_lcd.ySize)//下越界，移到屏幕上面显示。
		{
//			y=0;
//			x=0;
			return;
		}
		lattice=findLetter(&str[i],font);//获取该字符的点阵
		
		if(lattice)
		{
			//在屏幕上打印点阵
			lcd_print_lattic(lattice, width, font.size, x, y, foreground, background);
			x += width;//下一个字符的横坐标
		}
		else//没有的字符跳过去
		{
			x+=width;//下一个字符的横坐标。
		}
		i++;
		if(width==font.size)
		i++;
	}
}


//===========================
//RGB 24位色转 16位色
//我们的屏幕是支持24位色的，可惜cpu只支持16位
//所以颜色只能近似24位色，会存在色差
//一般16位颜色采用565
//===========================
uint16_t transfer24Bit_to_16Bit(uint32_t color)
{
	uint8_t R_Clor,G_Clor,B_Clor;
	R_Clor = ((color ) >> 16);
	G_Clor = ((color ) >> 8);
	B_Clor = color & 0xff;
	
	R_Clor >>= 3;	//取高5位
	G_Clor >>= 2;	//取高6位
	B_Clor >>= 3;	//取高5位
	
	return (R_Clor << 11) | (G_Clor << 5) | (B_Clor);
}

/*
*函数名称  lcd_show_num
*入口参数：num 需要打印的数字；x 起始行；y 起始列； foreground,前景色； background背景色； font,字体；type == 0不定长，type == 1定长显示；size，定长时的显示长度 < 10
*出口参数：无
*功能说明：在屏幕上打印数字的函数
注意：此函数最多可以显示10位数字
*/
void  lcd_show_num(uint32_t num,uint16_t x,uint16_t y ,uint16_t foreground, uint16_t background, const BX_FONT font ,uint8_t type, uint8_t size) 
{
	uint8_t i, len;
	char format[11];
	
	len = sprintf(format, "%u", num);
	format[len] = '\0';
	if(type != 0 && size < 11)
	{
		memcpy(&format[size-len], format, len);
		for(i = 0; i < size - len; i++)
		{
			format[i] = '0';
		}
	}
	
	lcd_show_string(format, x, y, foreground, background, font);
}


/*
时间显示
*/
void lcd_show_time(const WORD *time)
{
	char tmp[5];
	uint8_t len = 0, width = 0 ,total = 0;
	uint16_t value; 
	tmp[4] = 0;//结束符 
	
	width = time->font->size/2;
	
	//年
	if(calendar.w_year > 100)
	{
		value = calendar.w_year - (calendar.w_year/100) * 100;
	}
	else
	if(calendar.w_year < 100)
	{
		value = calendar.w_year;
	}
	len = sprintf(tmp, "%02u-", value);
	lcd_show_string(tmp, time->x, time->y, time->color, time->background, *(time->font));
	total += len;
	//月
	if(calendar.w_month <= 12 && calendar.w_month >= 1)
		value = calendar.w_month;
	else
		value = 0;
	len = sprintf(tmp, "%02u-", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//日
	if(calendar.w_date <= 31 && calendar.w_date >= 1)
		value = calendar.w_date;
	else
		value = 0;
	len = sprintf(tmp, "%02u ", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//时
	if(calendar.hour <= 23 )
		value = calendar.hour;
	else
		value = 0;
	len = sprintf(tmp, "%02u:", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//分
	if(calendar.min <= 59 )
		value = calendar.min;
	else
		value = 0;
	len = sprintf(tmp, "%02u", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));

	//秒
//	sprintf(tmp, "%04u", calendar.sec);
	//星期
//	sprintf(tmp, "->%01u", calendar.week);
//	lcd_show_string(tmp, (time->x + len*time->font->size/2), time->y, time->color, time->background, *(time->font));
	
}


/************
显示图片
每一位的数据流都是颜色信息，而不像显示点阵，每一字节表示8个颜色字节
************/



/*
清屏
*/
void lcd_clear(void)
{
	lcd_fill_color(0,0,g_lcd.xSize, g_lcd.ySize, g_lcd.background);
}




/*
打印字符串
*/
void  lcd_show_character(const WORD *character)
{
	uint8_t i = 0, width = 0;
	uint32_t x = character->x, y = character->y;
	const uint8_t *lattice;//点阵
	

	while(character->character[i]!='\0')
	{
		if(character->character[i] & 0x80 ) 	
			width = character->font->size;		//汉字长宽相同(gbk 码中ASCII码 <= 127(0x7f))
		else				
			width = character->font->size / 2;	//如果是英文和数字，只占用一半宽度
		
		if(x + width > g_lcd.xSize)//右越界，移到下一行显示。
		{
			//这里直接返回算了
			return;
//			y+=font.size;
//			x=0;
		}
		if(y + character->font->size > g_lcd.ySize)//下越界，移到屏幕上面显示。
		{
			return;
			//这里直接返回算了
//			y=0;
//			x=0;
		}
		
		lattice = findLetter(&character->character[i], *(character->font));//获取该字符的点阵
		
		if(lattice){
			//在屏幕上打印点阵。
			lcd_print_lattic( lattice, width, character->font->size ,x, y, character->color, character->background);
			x += width;//下一个字符的横坐标。
		}
		else
		{
			x += width;//下一个字符的横坐标。
		}

		i++;
		if(width == character->font->size)
		i++;
	}
//		tm12864_WriteCmd(0x36);//关图形显示
}

/*
清除字符串
其实就是用空白字符刷屏
*/
void  lcd_clear_character(const WORD *character)
{
	uint8_t i = 0, width = 0;
	uint32_t x = character->x, y = character->y;
	const uint8_t *lattice;//点阵
	const char tmp = ' ';

	while(character->character[i]!='\0')
	{
		if(character->character[i] & 0x80 )
		{			
			width = character->font->size;		//汉字长宽相同(gbk 码中ASCII码 <= 127(0x7f))
		}
		else
		{			
			width = character->font->size / 2;	//如果是英文和数字，只占用一半宽度
		}
		
		if(x + width > g_lcd.xSize)//右越界，移到下一行显示。
		{
			//这里直接返回算了
			return;
//			y+=font.size;
//			x=0;
		}
		if(y + character->font->size > g_lcd.ySize)//下越界，移到屏幕上面显示。
		{
			return;
			//这里直接返回算了
//			y=0;
//			x=0;
		}
		
		lattice = findLetter(&tmp, *(character->font));//寻找空白字符 
		
		if(lattice)
		{
			if(width == character->font->size)
			{
				lcd_print_lattic( lattice, width/2, character->font->size ,x, y, character->color, character->background);
				x += (width/2);//半个字符宽度
				lcd_print_lattic( lattice, width/2, character->font->size ,x, y, character->color, character->background);
				x += (width/2);//半个字符宽度
			}
			else
			{
				lcd_print_lattic( lattice, width, character->font->size ,x, y, character->color, character->background);
				x += width;//下一个字符的横坐标。
			}
		}
		else
		{
			x += width;//下一个字符的横坐标。
		}

		i++;
		if(width == character->font->size)
		i++;
	}
}



/*
显示器初始化
*/
void lcd_init(void)
{
	word_init();
	lcd_set_par();					//设置屏幕参数
	bsp_lcd_hard_congfigure();		//设备配置
	bsp_lcd_set_directrion(0, (uint16_t*)&g_lcd.xSize, (uint16_t*)&g_lcd.ySize);		//屏幕方向设置
	lcd_clear(); 
}


