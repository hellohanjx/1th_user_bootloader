/*
常用短语放到这里来
这个可以作为一个配置文件
*/
#include "word.h"
#include "lcd_header.h"


/*============================================================================================
单色屏幕
============================================================================================*/

#if ((SCREEN == TM12864Z) || (SCREEN == TM12864_NO_FONT))

/*
警告提示信息
*/	
const WORD chs_prompt[] = {
	
{"升级请插U盘  ", 0, 0, YELLOW, BLUE, &font16, 0, 0},	//0
{"\0", 1000, 0, 	YELLOW, BLACK, &font16, 0, 0},	//1 不显示
{"正在检测U盘  ", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//2
{"CH376 ERROR", 0, 2, RED, BLACK, &font16, 8*13, 2},	//3
{"未插入U盘   ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//4
{"检测到U盘   ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//5
{"U盘正常     ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//6
{"U盘断开     ", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//7
{"未知设备,请移除", 0, 2, YELLOW, BLUE, &font16, 0, 0},	//8
{"这里显示厂商信息            ", 1000, 120, YELLOW, BLUE, &font16, 0, 0},	//9	不显示
{"未找到升级文件 ", 0, 2, YELLOW, BLUE, &font16, 1000, 180},			//10
{"已找到升级文件 ", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//11
{"U盘移除重新检测", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//12
{"异常重新检测", 0, 2, YELLOW, BLUE, &font16, 8*12, 2},		//13	
{"正在升级请稍候", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//14
{"升级失败请重试", 0, 2, YELLOW, BLUE, &font16, 0, 0},		//15
{"      Byte写入  ", 	0, 4, YELLOW, BLUE, &font16, 0, 4},		//16
{"秒后跳转", 	2*8, 6, YELLOW, BLUE, &font16, 0, 6},		//17
{"正在启动...     ", 0, 6, YELLOW, BLUE, &font16, 0, 0},		//18
{"HardFault_Handler", 0, 0, RED, BLACK, &font16, 0, 0},		//19
{"RTC NO START     ", 0, 0, YELLOW, BLACK, &font16, 0, 0},	//20
{"电源复位", 	1000, 0, RED, BLUE, &font16, 0, 0},			//21 不显示
{"按下复位键", 	1000, 0, RED, BLUE, &font16, 0, 0},			//22 不显示
{"1.升级主板程序  ", 0, 2, RED, BLACK, &font16, 0, 0},			//23
{"2.升级驱动板程序", 0, 4, RED, BLACK, &font16, 0, 0},			//24
{"请选择升级目标  ", 0, 0, RED, BLACK, &font16, 0, 0},			//25
{"                ", 0, 6, RED, BLACK, &font16, 0, 0},			//26
{"                ", 0, 4, RED, BLACK, &font16, 0, 0},			//27
{"                ", 0, 2, RED, BLACK, &font16, 0, 0},			//28
{"驱动板升级成功  ", 0, 6, RED, BLACK, &font16, 0, 0},			//29

};




/*==============================================================================================
彩色屏幕，横屏
==============================================================================================*/
#elif ((SCREEN == SCREENILI9806) || (SCREEN == SCREENRA8875))


const WORD chs_prompt[] = {
	
{"正在启动                   ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//0
{"\0", 30, 0, 	YELLOW, BLACK, &font48, 0, 0},	//1
{"正在检测U盘                ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//2
{"CH376 ERROR,error Code ", 30, 120, RED, BLACK, &font48, 650, 120},	//3
{"未插入U盘                  ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//4
{"检测到U盘                  ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//5
{"U盘正常                    ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//6
{"U盘断开                    ", 30, 60, 	YELLOW, BLACK, &font48, 0, 0},	//7
{"未知设备,请移除             ", 30, 60, YELLOW, BLACK, &font48, 0, 0},	//8
{"这里显示厂商信息            ", 30, 120, YELLOW, BLACK, &font48, 0, 0},	//9
{"未找升级到文件 ", 30, 180, YELLOW, BLACK, &font48, 400, 180},			//10
{"已找到升级文件              ", 30, 180, YELLOW, BLACK, &font48, 0, 0},		//11
{"U盘移除重新检测             ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//12
{"发生异常重新检测            ", 30, 240, YELLOW, BLACK, &font48, 550, 240},	//13
{"正在升级请稍候              ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//14
{"升级失败请重试              ", 30, 240, YELLOW, BLACK, &font48, 0, 0},		//15
{"Byte写入Flash              ", 200, 360, YELLOW, BLACK, &font48, 30, 360},	//16
{"秒后跳转   (支持FAT格式U盘) ", 80, 360, YELLOW, BLACK, &font48, 30, 360},	//17
{"正在启动...     ", 30, 420, YELLOW, BLACK, &font48, 0, 0},		//18
{"HardFault_Handler          ", 30, 0, RED, BLACK, &font48, 0, 0},			//19
{"RTC NO START               ", 30, 120, YELLOW, BLACK, &font48, 0, 0},		//20
{"电源复位                   ", 30, 0, RED, BLACK, &font48, 0, 0},			//21
{"按下复位键                 ", 30, 0, RED, BLACK, &font48, 0, 0},			//22
{"1.升级主板程序  ", 30, 0, RED, BLACK, &font48, 0, 0},			//23
{"2.升级驱动板程序", 30, 0, RED, BLACK, &font48, 0, 0},			//24
{"请选择升级目标  ", 30, 0, RED, BLACK, &font48, 0, 0},			//25
{"                ", 30, 0, RED, BLACK, &font48, 0, 0},			//26
};


#endif






const WORD* pt_word; 
/*
字符串初始化
*/
void word_init(void)
{
	if(0)//英文
	{
		
		
	}
	else
	{
		pt_word = chs_prompt;
	}
	
}


