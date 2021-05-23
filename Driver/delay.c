/**
  * @file    Gpio.c 
  * @author  WB R&D Team - openmcu666
  * @version V1.0
  * @date    2016.05.04
  * @brief   systick
  */
#include "delay.h"

//static	uint8_t  sysclk = 72;                             //默认系统时钟为72MHz


//==============================
//下面两个是粗糙的延时函数
//根据原子哥测算，stm32 72M时一条指令执行时间大约70ns
//==============================
//微秒延时
void delay_us(uint16_t time) 
{         
	uint16_t i=0;      
	while(time--)    
	{        
		i=10;      
		while(i--);         
	} 
}  
//毫秒延时
void delay_ms(uint16_t time) 
{         
	uint16_t i=0;      
	while(time--)    
	{        
		i=12000; 
		while(i--);    
	} 
}

//==============================
//精确延时
//c采用系统滴答计时器
//==============================

/**
  * @brief  基准延时初始化，使用SysTick;
  *         SysTick时钟源由HCLK/8提供，当系统频率为72MHz时
  *         最小计数周期为1/9MHz,计满9次为1us，fac_us以9为单位        
  * @param  None
  * @retval None
  */
void Delay_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);     //选择滴答定时器的时钟源

}


//==============================
//延时单位 ms
//==============================
void Delay_ms(uint32_t nms)
{	 		  	  
	uint32_t temp;	
	while(nms-- > 0)
	{		
		SysTick->LOAD=9000;                        				//时间加载(SysTick->LOAD为24bit)
		SysTick->VAL =0x00;                                        //清空计数器 
		SysTick->CTRL=0x01 ;                                      //开始倒数   
		
		do{
			temp = SysTick->CTRL;
		}while(temp&0x01 && !(temp&(1<<16)));                         //等待时间到达 
	}	
	SysTick->CTRL=0x00;                                         //关闭计数器	 
	SysTick->VAL =0x00;                                         //清空计数器	 	    
} 

/**
  * @brief  实现微秒级延时，最大延时1864ms
  *         这两个函数是通过寄存器配置的，没有对应的固件库函数，参考misc.h
  * @param  us(number us) 延时微秒数 最大1864135
  * @retval None
  */

void Delay_us(uint32_t nus)
{		
	uint32_t temp;	   
	
	
	SysTick->VAL=0x00;                                    //清空计数器 
	
	SysTick->LOAD=(nus*9 & 0xffffff);                     //时间加载 fac_us=9,走9次就是1us	 		 
	SysTick->CTRL=0x01 ;                                  //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));                     //等待时间到达  
	SysTick->CTRL=0x00;                                    //关闭计数器
	SysTick->VAL =0x00;                                    //清空计数器	
}
