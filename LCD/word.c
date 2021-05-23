/*
���ö���ŵ�������
���������Ϊһ�������ļ�
*/
#include "word.h"
#include "lcd_header.h"


/*============================================================================================
��ɫ��Ļ
============================================================================================*/

#if ((SCREEN == TM12864Z) || (SCREEN == TM12864_NO_FONT))

/*
������ʾ��Ϣ
*/	
const WORD chs_prompt[] = {
	
{"�������U��  ", 0, 0, YELLOW, BLUE, &font16, 0, 0},	//0
{"\0", 1000, 0, 	YELLOW, BLACK, &font16, 0, 0},	//1 ����ʾ
{"���ڼ��U��  ", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//2
{"CH376 ERROR", 0, 2, RED, BLACK, &font16, 8*13, 2},	//3
{"δ����U��   ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//4
{"��⵽U��   ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//5
{"U������     ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//6
{"U�̶Ͽ�     ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//7
{"δ֪�豸,���Ƴ�", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//8
{"������ʾ������Ϣ            ", 1000, 120, YELLOW, BLUE, &font16, 0, 0},	//9	����ʾ
{"δ�ҵ������ļ� ", 0, 2, YELLOW, BLUE, &font16, 1000, 180},			//10
{"���ҵ������ļ� ", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//11
{"U���Ƴ����¼��", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//12
{"�쳣���¼��", 0, 2, YELLOW, BLUE, &font16, 8*12, 2},		//13	
{"�����������Ժ�", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//14
{"����ʧ��������", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//15
{"      Byteд��  ", 	0, 4, YELLOW, BLUE, &font16, 0, 4},		//16
{"�����ת", 	2*8, 6, YELLOW, BLUE, &font16, 0, 6},		//17
{"��������...     ", 0, 6, YELLOW, BLUE, &font16, 0, 0},		//18
{"HardFault_Handler", 0, 0, RED, BLACK, &font16, 0, 0},		//19
{"RTC NO START     ", 0, 0, YELLOW, BLACK, &font16, 0, 0},	//20
{"��Դ��λ", 	1000, 0, RED, BLUE, &font16, 0, 0},			//21 ����ʾ
{"���¸�λ��", 	1000, 0, RED, BLUE, &font16, 0, 0},			//22 ����ʾ
{"1.�����������  ", 0, 2, RED, BLACK, &font16, 0, 0},			//23
{"2.�������������", 0, 4, RED, BLACK, &font16, 0, 0},			//24
{"��ѡ������Ŀ��  ", 0, 0, RED, BLACK, &font16, 0, 0},			//25
{"                ", 0, 6, RED, BLACK, &font16, 0, 0},			//26
{"                ", 0, 4, RED, BLACK, &font16, 0, 0},			//27
{"                ", 0, 2, RED, BLACK, &font16, 0, 0},			//28
{"�����������ɹ�  ", 0, 6, RED, BLACK, &font16, 0, 0},			//29

};




/*==============================================================================================
��ɫ��Ļ������
==============================================================================================*/
#elif ((SCREEN == SCREENILI9806) || (SCREEN == SCREENRA8875))


const WORD chs_prompt[] = {
	
{"��������                   ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//0
{"\0", 30, 0, 	YELLOW, BLACK, &font48, 0, 0},	//1
{"���ڼ��U��                ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//2
{"CH376 ERROR,error Code ", 30, 120, RED, BLACK, &font48, 650, 120},	//3
{"δ����U��                  ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//4
{"��⵽U��                  ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//5
{"U������                    ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//6
{"U�̶Ͽ�                    ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//7
{"δ֪�豸,���Ƴ�             ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//8
{"������ʾ������Ϣ            ", 30, 120, YELLOW, BLACK, &font48, 0, 0},	//9
{"δ���������ļ� ", 30, 180, YELLOW, BLACK, &font48, 400, 180},			//10
{"���ҵ������ļ�              ", 30, 180, YELLOW, BLACK, &font48, 0, 0},		//11
{"U���Ƴ����¼��             ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//12
{"�����쳣���¼��            ", 30, 240, YELLOW, BLACK, &font48, 550, 240},	//13
{"�����������Ժ�              ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//14
{"����ʧ��������              ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//15
{"Byteд��Flash              ", 200, 360, YELLOW, BLACK, &font48, 30, 360},	//16
{"�����ת   (֧��FAT��ʽU��) ", 80, 360, YELLOW, BLACK, &font48, 30, 360},	//17
{"��������...     ", 30, 420, YELLOW, BLACK, &font48, 0, 0},		//18
{"HardFault_Handler          ", 30, 0, RED, BLACK, &font48, 0, 0},			//19
{"RTC NO START               ", 30, 120, YELLOW, BLACK, &font48, 0, 0},		//20
{"��Դ��λ                   ", 30, 0, RED, BLACK, &font48, 0, 0},			//21
{"���¸�λ��                 ", 30, 0, RED, BLACK, &font48, 0, 0},			//22
{"1.�����������  ", 30, 0, RED, BLACK, &font48, 0, 0},			//23
{"2.�������������", 30, 0, RED, BLACK, &font48, 0, 0},			//24
{"��ѡ������Ŀ��  ", 30, 0, RED, BLACK, &font48, 0, 0},			//25
{"                ", 30, 0, RED, BLACK, &font48, 0, 0},			//26
};


#endif






const WORD* pt_word; 
/*
�ַ�����ʼ��
*/
void word_init(void)
{
	if(0)//Ӣ��
	{
		
		
	}
	else
	{
		pt_word = chs_prompt;
	}
	
}


