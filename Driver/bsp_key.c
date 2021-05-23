/*
��������

*/



/*
Լ������
KEY1~KEY4 �����У�3.3v����
key4 ��һ��
key3 �ڶ���
key2 ������
key1 ������

KEY5~KEY8 ������
key5 ��һ��
key6 �ڶ���
key7 ������
key8 ��4��
*/
//12������
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
						 | RCC_APB2Periph_AFIO , DISABLE);//ע���������ʱ����ʵֻ�� I2C1 ������
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
					 | RCC_APB2Periph_AFIO , ENABLE);//ע���������ʱ����ʵֻ�� I2C1 ������
	//������PA8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);

	//DOOR PA1
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;//����ĳ��������ֹ����
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
	
	//��������� KEY5,KEY6,KEY7,KEY8.
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF,&GPIO_InitStructure);
  
	//��������� KEY1,KEY2,KEY3,KEY4
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}


/*
��ɨ��
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
	bool lock_key ;	//��ֵ����
	uint8_t key_row, key_line;	//��¼��ʱֵ
	uint8_t curRow, curLine ; 	//������¼���µļ�ֵ
	uint8_t rs = 0xff;
	
	
	key_line = curRow = curLine = 0xff;
	lock_key = FALSE;
	for(key_row = 0;key_row < 4; key_row++ )
	{
		GPIO_WriteBit(GPIOF,pin[key_row],Bit_SET);//KET5~KEY8 ����
	}
	
	
			for(key_row = 0; key_row < 4; key_row++)
			{
				GPIO_WriteBit(GPIOF,pin[key_row],Bit_RESET);//KEY5~KEY8 ��������
				delay_ms(10);	//�ȴ���ƽƽ��
				curRow = key_row; 	//��¼��ǰ���͵���
				curLine = key_line = keyBoardHscan();	//��¼�а���
				if(key_line != 0xff)			//�а��¶���
				{
					delay_ms(10);				//ȥ���������Ե��ڰ�����������
					key_line = keyBoardHscan();
					if(curLine == key_line )//�������ǰ�水�µļ�
					{
						BEEP(ON);
						delay_ms(100);
						BEEP(OFF);
						rs = keyValue[curRow][curLine];
						lock_key = TRUE;
						while(lock_key)//��ס����������������ſ�������ɨ��
						{
							if(keyBoardHscan() == 0xff)//��������
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
				GPIO_WriteBit(GPIOF,pin[key_row],Bit_SET);//����������
			}
	return rs;
}


