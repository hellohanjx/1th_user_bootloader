/*
lcd bsp֧�ְ�
*/

#include "lcd_drv.h" 
#include "lcd_header.h"  
#include "font_header.h"

/*
��Ļ��Ϣȫ�ֱ���
*/
 volatile SCREEN_INFO g_lcd;  

/**********
����ӿں���
***********/

/*
���ô�ӡ�ַ���ʱ�ı�����ɫ
*/
void lcd_set_background(uint16_t background)
{
	g_lcd.background = background;
}

/*
���ô�ӡ�ַ���ʱ��ǰ����ɫ
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
lcd ��������
��ʾ������Ӧ���ǲ�������ɫ�������,�ֱ����ǹ̶���
*/
static void lcd_set_par(void)
{
	g_lcd.background = BLACK;				//���ñ���ɫ
	g_lcd.foreground = YELLOW;				//����ǰ��ɫ
	g_lcd.hightLightBackground = RED;		//���ø�������ɫ
	g_lcd.hightLightforeground = BLACK;		//���ø���ǰ��ɫ
	g_lcd.promptColor = YELLOW;				//������ʾ��Ϣ��ɫ
	g_lcd.waringColor = RED;				//���þ�����Ϣ��ɫ
	g_lcd.xSize = bsp_get_xsize();
	g_lcd.ySize = bsp_get_ysize();
}




//===========================
//����ĸ�����ǲ�������
//������
//key �ַ�
//font ����
//===========================
static const uint8_t* findLetter(const char *key,const BX_FONT font)
{
	uint16_t i=0;
	const uint8_t* lattice; //��Ҫ�ҵ��ĵ���ָ�롣
	const char* index;
	index = font.index;
	lattice = font.lattice;
	do
	{
		if(key[0] & 0x80)//�����������˫�ֽ�
		{
			if(index[i] & 0x80)//���������ǰҲ��˫�ֽ�
			{
				if(key[0]==index[i] && key[1]==index[i+1])//���ƥ��ɹ�
				{
					return lattice;//���ص���
				}
				else//ƥ�䲻�ɹ�
				{
					lattice+=font.size*font.size/8;	//2�ֽڱ�ʾ���ַ�ռ�õ��ֽ���
					i+=2;//����������
				}
			}
			else //��������ǵ��ֽ�
			{
				//����������
				lattice+=font.size*font.size/16;
				i++;
			}
		}
		else//����������ǵ��ֽ�
		{
			if(index[i] & 0x80)
			{
				//�����ǰ������˫�ֽ�������������������һ��
				lattice+=font.size*font.size/8;			
				i+=2;//����������
			}
			else
			{
				if(key[0]==index[i])//���ƥ��ɹ�
				{
					return lattice;//���ص���
				}
				else
					lattice+=font.size*font.size/16;	//1�ֽڱ�ʾ���ַ�ռ�õ��ֽ���
				i++;
			}
		}
	}while(index[i]!='\0');
	
	return FALSE;	//�ҵ����ص�ַ���Ҳ�������0
	
}



/*
�ṩȫ�����ַ�����ӡ����
��Σ�str ��Ҫ��ӡ���ַ�����x ��ʼ��   y ��ʼ�У� font ���壬foreground ǰ��ɫ��������ɫ����background ����ɫ�����屳����
���Σ���
*/
void  lcd_show_string(const char* str,uint16_t x ,uint16_t y, uint16_t foreground, uint16_t background, const BX_FONT font)
{
	uint8_t i;
	uint8_t width;
	const uint8_t *lattice;//����
	i=0;
	while(str[i]!='\0')
	{
		if(str[i] & 0x80 ) 	//���ֳ�����ͬ(gbk ����ASCII�� <= 127(0x7f))
			width=font.size;
		else				//�����Ӣ�ĺ����֣�ֻռ��һ����
			width=font.size/2;
		
		if(x+width>g_lcd.xSize)//��Խ�磬�Ƶ���һ����ʾ��
		{
//			y+=font.size;
//			x=0;
			return;
		}
		if(y+font.size>g_lcd.ySize)//��Խ�磬�Ƶ���Ļ������ʾ��
		{
//			y=0;
//			x=0;
			return;
		}
		lattice=findLetter(&str[i],font);//��ȡ���ַ��ĵ���
		
		if(lattice)
		{
			//����Ļ�ϴ�ӡ����
			lcd_print_lattic(lattice, width, font.size, x, y, foreground, background);
			x += width;//��һ���ַ��ĺ�����
		}
		else//û�е��ַ�����ȥ
		{
			x+=width;//��һ���ַ��ĺ����ꡣ
		}
		i++;
		if(width==font.size)
		i++;
	}
}


//===========================
//RGB 24λɫת 16λɫ
//���ǵ���Ļ��֧��24λɫ�ģ���ϧcpuֻ֧��16λ
//������ɫֻ�ܽ���24λɫ�������ɫ��
//һ��16λ��ɫ����565
//===========================
uint16_t transfer24Bit_to_16Bit(uint32_t color)
{
	uint8_t R_Clor,G_Clor,B_Clor;
	R_Clor = ((color ) >> 16);
	G_Clor = ((color ) >> 8);
	B_Clor = color & 0xff;
	
	R_Clor >>= 3;	//ȡ��5λ
	G_Clor >>= 2;	//ȡ��6λ
	B_Clor >>= 3;	//ȡ��5λ
	
	return (R_Clor << 11) | (G_Clor << 5) | (B_Clor);
}

/*
*��������  lcd_show_num
*��ڲ�����num ��Ҫ��ӡ�����֣�x ��ʼ�У�y ��ʼ�У� foreground,ǰ��ɫ�� background����ɫ�� font,���壻type == 0��������type == 1������ʾ��size������ʱ����ʾ���� < 10
*���ڲ�������
*����˵��������Ļ�ϴ�ӡ���ֵĺ���
ע�⣺�˺�����������ʾ10λ����
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
ʱ����ʾ
*/
void lcd_show_time(const WORD *time)
{
	char tmp[5];
	uint8_t len = 0, width = 0 ,total = 0;
	uint16_t value; 
	tmp[4] = 0;//������ 
	
	width = time->font->size/2;
	
	//��
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
	//��
	if(calendar.w_month <= 12 && calendar.w_month >= 1)
		value = calendar.w_month;
	else
		value = 0;
	len = sprintf(tmp, "%02u-", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//��
	if(calendar.w_date <= 31 && calendar.w_date >= 1)
		value = calendar.w_date;
	else
		value = 0;
	len = sprintf(tmp, "%02u ", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//ʱ
	if(calendar.hour <= 23 )
		value = calendar.hour;
	else
		value = 0;
	len = sprintf(tmp, "%02u:", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));
	total += len;
	//��
	if(calendar.min <= 59 )
		value = calendar.min;
	else
		value = 0;
	len = sprintf(tmp, "%02u", value);
	lcd_show_string(tmp, time->x + total*width, time->y, time->color, time->background, *(time->font));

	//��
//	sprintf(tmp, "%04u", calendar.sec);
	//����
//	sprintf(tmp, "->%01u", calendar.week);
//	lcd_show_string(tmp, (time->x + len*time->font->size/2), time->y, time->color, time->background, *(time->font));
	
}


/************
��ʾͼƬ
ÿһλ��������������ɫ��Ϣ����������ʾ����ÿһ�ֽڱ�ʾ8����ɫ�ֽ�
************/



/*
����
*/
void lcd_clear(void)
{
	lcd_fill_color(0,0,g_lcd.xSize, g_lcd.ySize, g_lcd.background);
}




/*
��ӡ�ַ���
*/
void  lcd_show_character(const WORD *character)
{
	uint8_t i = 0, width = 0;
	uint32_t x = character->x, y = character->y;
	const uint8_t *lattice;//����
	

	while(character->character[i]!='\0')
	{
		if(character->character[i] & 0x80 ) 	
			width = character->font->size;		//���ֳ�����ͬ(gbk ����ASCII�� <= 127(0x7f))
		else				
			width = character->font->size / 2;	//�����Ӣ�ĺ����֣�ֻռ��һ����
		
		if(x + width > g_lcd.xSize)//��Խ�磬�Ƶ���һ����ʾ��
		{
			//����ֱ�ӷ�������
			return;
//			y+=font.size;
//			x=0;
		}
		if(y + character->font->size > g_lcd.ySize)//��Խ�磬�Ƶ���Ļ������ʾ��
		{
			return;
			//����ֱ�ӷ�������
//			y=0;
//			x=0;
		}
		
		lattice = findLetter(&character->character[i], *(character->font));//��ȡ���ַ��ĵ���
		
		if(lattice){
			//����Ļ�ϴ�ӡ����
			lcd_print_lattic( lattice, width, character->font->size ,x, y, character->color, character->background);
			x += width;//��һ���ַ��ĺ����ꡣ
		}
		else
		{
			x += width;//��һ���ַ��ĺ����ꡣ
		}

		i++;
		if(width == character->font->size)
		i++;
	}
//		tm12864_WriteCmd(0x36);//��ͼ����ʾ
}

/*
����ַ���
��ʵ�����ÿհ��ַ�ˢ��
*/
void  lcd_clear_character(const WORD *character)
{
	uint8_t i = 0, width = 0;
	uint32_t x = character->x, y = character->y;
	const uint8_t *lattice;//����
	const char tmp = ' ';

	while(character->character[i]!='\0')
	{
		if(character->character[i] & 0x80 )
		{			
			width = character->font->size;		//���ֳ�����ͬ(gbk ����ASCII�� <= 127(0x7f))
		}
		else
		{			
			width = character->font->size / 2;	//�����Ӣ�ĺ����֣�ֻռ��һ����
		}
		
		if(x + width > g_lcd.xSize)//��Խ�磬�Ƶ���һ����ʾ��
		{
			//����ֱ�ӷ�������
			return;
//			y+=font.size;
//			x=0;
		}
		if(y + character->font->size > g_lcd.ySize)//��Խ�磬�Ƶ���Ļ������ʾ��
		{
			return;
			//����ֱ�ӷ�������
//			y=0;
//			x=0;
		}
		
		lattice = findLetter(&tmp, *(character->font));//Ѱ�ҿհ��ַ� 
		
		if(lattice)
		{
			if(width == character->font->size)
			{
				lcd_print_lattic( lattice, width/2, character->font->size ,x, y, character->color, character->background);
				x += (width/2);//����ַ����
				lcd_print_lattic( lattice, width/2, character->font->size ,x, y, character->color, character->background);
				x += (width/2);//����ַ����
			}
			else
			{
				lcd_print_lattic( lattice, width, character->font->size ,x, y, character->color, character->background);
				x += width;//��һ���ַ��ĺ����ꡣ
			}
		}
		else
		{
			x += width;//��һ���ַ��ĺ����ꡣ
		}

		i++;
		if(width == character->font->size)
		i++;
	}
}



/*
��ʾ����ʼ��
*/
void lcd_init(void)
{
	word_init();
	lcd_set_par();					//������Ļ����
	bsp_lcd_hard_congfigure();		//�豸����
	bsp_lcd_set_directrion(0, (uint16_t*)&g_lcd.xSize, (uint16_t*)&g_lcd.ySize);		//��Ļ��������
	lcd_clear(); 
}


