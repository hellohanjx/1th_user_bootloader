/**
  * @file    Gpio.c 
  * @author  WB R&D Team - openmcu666
  * @version V1.0
  * @date    2016.05.04
  * @brief   systick
  */
#include "delay.h"

//static	uint8_t  sysclk = 72;                             //Ĭ��ϵͳʱ��Ϊ72MHz


//==============================
//���������Ǵֲڵ���ʱ����
//����ԭ�Ӹ���㣬stm32 72Mʱһ��ָ��ִ��ʱ���Լ70ns
//==============================
//΢����ʱ
void delay_us(uint16_t time) 
{         
	uint16_t i=0;      
	while(time--)    
	{        
		i=10;      
		while(i--);         
	} 
}  
//������ʱ
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
//��ȷ��ʱ
//c����ϵͳ�δ��ʱ��
//==============================

/**
  * @brief  ��׼��ʱ��ʼ����ʹ��SysTick;
  *         SysTickʱ��Դ��HCLK/8�ṩ����ϵͳƵ��Ϊ72MHzʱ
  *         ��С��������Ϊ1/9MHz,����9��Ϊ1us��fac_us��9Ϊ��λ        
  * @param  None
  * @retval None
  */
void Delay_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);     //ѡ��δ�ʱ����ʱ��Դ

}


//==============================
//��ʱ��λ ms
//==============================
void Delay_ms(uint32_t nms)
{	 		  	  
	uint32_t temp;	
	while(nms-- > 0)
	{		
		SysTick->LOAD=9000;                        				//ʱ�����(SysTick->LOADΪ24bit)
		SysTick->VAL =0x00;                                        //��ռ����� 
		SysTick->CTRL=0x01 ;                                      //��ʼ����   
		
		do{
			temp = SysTick->CTRL;
		}while(temp&0x01 && !(temp&(1<<16)));                         //�ȴ�ʱ�䵽�� 
	}	
	SysTick->CTRL=0x00;                                         //�رռ�����	 
	SysTick->VAL =0x00;                                         //��ռ�����	 	    
} 

/**
  * @brief  ʵ��΢�뼶��ʱ�������ʱ1864ms
  *         ������������ͨ���Ĵ������õģ�û�ж�Ӧ�Ĺ̼��⺯�����ο�misc.h
  * @param  us(number us) ��ʱ΢���� ���1864135
  * @retval None
  */

void Delay_us(uint32_t nus)
{		
	uint32_t temp;	   
	
	
	SysTick->VAL=0x00;                                    //��ռ����� 
	
	SysTick->LOAD=(nus*9 & 0xffffff);                     //ʱ����� fac_us=9,��9�ξ���1us	 		 
	SysTick->CTRL=0x01 ;                                  //��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));                     //�ȴ�ʱ�䵽��  
	SysTick->CTRL=0x00;                                    //�رռ�����
	SysTick->VAL =0x00;                                    //��ռ�����	
}
