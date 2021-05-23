/**
  * @file    Gpio.c 
  * @author  WB R&D Team - openmcu666
  * @version V1.0
  * @date    2016.05.04
  * @brief   GPIO Driver
  */
#include "Gpio.h"

/**
  * @brief  初始化LED管脚
  * @param  None
  * @retval None
  */
void LED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	//初始化led_breath 与 led_usb
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC , GPIO_Pin_4 | GPIO_Pin_5);
	//GPIOC->BRR = GPIO_Pin_4 | GPIO_Pin_5;
	
	//初始化led_usb
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	//GPIOA->BRR = GPIO_Pin_11;
	
}

//================================
//打开 led 指示灯
//================================
void LED_Toggle_On(uint8_t led)
{
	switch(led)
	{	
		case 1: GPIOC->BRR = GPIO_Pin_4;
			break;
		case 2: GPIOC->BRR = GPIO_Pin_5;
			break;
		case 3: GPIOA->BRR = GPIO_Pin_11;
			break;
	}
}
//=================================
//关闭 led 指示灯
//=================================
void LED_Toggle_Off(uint8_t led)
{
	switch(led)
	{	
		case 1: GPIOC->BSRR = GPIO_Pin_4;
			break;
		case 2: GPIOC->BSRR = GPIO_Pin_5;
			break;
		case 3: GPIOA->BSRR = GPIO_Pin_11;
			break;
	}
}


/**
  * @brief  初始化蜂鸣器控制管脚PD3
  * @param  None
  * @retval None
  */
void BEEP_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	
  RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
	
  GPIO_ResetBits(BEEP_GPIO_PORT,BEEP_GPIO_PIN);

}

/**
  * @brief  初始化按键检测引脚
  * @param  None
  * @retval None
  */
void Key_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(KEY2_3_GPIO_CLK | KEY4_GPIO_CLK | KEY5_GPIO_CLK , ENABLE);

 	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN | KEY3_GPIO_PIN;	  //PA0 PA8设置为输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY2_3_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = KEY4_GPIO_PIN;				              //PC13设置为输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY4_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = KEY5_GPIO_PIN;					              //PD3设置为输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(KEY5_GPIO_PORT, &GPIO_InitStructure); 
	
}
