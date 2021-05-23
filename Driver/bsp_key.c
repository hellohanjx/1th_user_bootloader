/*
键盘驱动

*/



/*
约定规则：
KEY1~KEY4 键盘列，3.3v供电
key4 第一列
key3 第二列
key2 第三列
key1 第四列

KEY5~KEY8 键盘行
key5 第一行
key6 第二行
key7 第三行
key8 第4行
*/
//12键键盘
//1 2 3
//4 5 6
//7 8 9
//* 0 #
//===============================
#include "bsp_key.h" 

static const uint8_t keyValue[4][4]={
	{'1','2','3','*'},
	{'4','5','6','0'},
	{'7','8','9','#'}, 
	{'*','0','#','#'},
};


static const uint16_t pin[]={GPIO_Pin_10, GPIO_Pin_9, GPIO_Pin_8, GPIO_Pin_7};

void key_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	//WAKEUP PA0
//	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA,&GPIO_InitStructure);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA 
						 | RCC_APB2Periph_GPIOB 
						 | RCC_APB2Periph_GPIOC 
						 | RCC_APB2Periph_GPIOD 
						 | RCC_APB2Periph_GPIOE 
						 | RCC_APB2Periph_GPIOF 
						 | RCC_APB2Periph_GPIOG 
						 | RCC_APB2Periph_AFIO , DISABLE);//注意这个复用时钟其实只在 I2C1 用了下
	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_DeInit(GPIOD);
	GPIO_DeInit(GPIOE);
	GPIO_DeInit(GPIOF);
	GPIO_DeInit(GPIOG);
	GPIO_AFIODeInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA 
					 | RCC_APB2Periph_GPIOB 
					 | RCC_APB2Periph_GPIOC 
					 | RCC_APB2Periph_GPIOD 
					 | RCC_APB2Periph_GPIOE 
					 | RCC_APB2Periph_GPIOF 
					 | RCC_APB2Periph_GPIOG 
					 | RCC_APB2Periph_AFIO , ENABLE);//注意这个复用时钟其实只在 I2C1 用了下
	//蜂鸣器PA8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);

	//DOOR PA1
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;//这里改成输出，防止串扰
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//USER1 PE2
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//USER2,3,4 PB7,PB6,PB5
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//键盘输出脚 KEY5,KEY6,KEY7,KEY8.
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF,&GPIO_InitStructure);
  
	//键盘输出脚 KEY1,KEY2,KEY3,KEY4
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}


/*
列扫描
*/
static uint8_t keyBoardHscan(void)
{
	uint8_t inputData;
	inputData = GPIO_ReadInputData(GPIOC);
	inputData &= 0x0F;
	switch(inputData)
	{
		case ____XXX_:
			return 0;
		case ____XX_X:
			return 1;
		case ____X_XX:
			return 2;
		case _____XXX:
			return 3;
		case ____XX__:
			return 4;
		case ____X__X:
			return 5;
		case ____X_X_:
			return 6;
		case _____XX_:
			return 7;
		default:
			return 0xFF;
	}
}



/*
*/
uint8_t key_run(void)
{
	bool lock_key ;	//键值锁定
	uint8_t key_row, key_line;	//记录临时值
	uint8_t curRow, curLine ; 	//用来记录按下的键值
	uint8_t rs = 0xff;
	
	
	key_line = curRow = curLine = 0xff;
	lock_key = FALSE;
	for(key_row = 0;key_row < 4; key_row++ )
	{
		GPIO_WriteBit(GPIOF,pin[key_row],Bit_SET);//KET5~KEY8 拉高
	}
	
	
			for(key_row = 0; key_row < 4; key_row++)
			{
				GPIO_WriteBit(GPIOF,pin[key_row],Bit_RESET);//KEY5~KEY8 逐行拉低
				delay_ms(10);	//等待电平平稳
				curRow = key_row; 	//记录当前拉低的行
				curLine = key_line = keyBoardHscan();	//记录列按键
				if(key_line != 0xff)			//有按下动作
				{
					delay_ms(10);				//去抖动，可以调节按键的灵敏度
					key_line = keyBoardHscan();
					if(curLine == key_line )//如果还是前面按下的键
					{
						BEEP(ON);
						delay_ms(100);
						BEEP(OFF);
						rs = keyValue[curRow][curLine];
						lock_key = TRUE;
						while(lock_key)//锁住这个按键动作，不放开不重新扫描
						{
							if(keyBoardHscan() == 0xff)//按键弹起
							{
								lock_key = FALSE;
							}
							else
							{
								delay_ms(20);
							}
						}
					}
				}
				GPIO_WriteBit(GPIOF,pin[key_row],Bit_SET);//检测的行拉高
			}
	return rs;
}


