/*
键盘驱动头文件

*/

#ifndef _KEY_H_
#define _KEY_H_

#include "stdint.h"
#include "stm32f10x_gpio.h"


#define WAKE_UP		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) 	//pA0

#define USER_1		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) 	//PE2
#define USER_2		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7) 	//PB7
#define USER_3		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6) 	//PB6
#define USER_4		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) 	//PB5

#define KEY1_R		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3) 	//PC3
#define KEY2_R		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2) 	//PC2
#define KEY3_R		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1) 	//PC1
#define KEY4_R		GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0) 	//PC0

#define KEY5_W(x)		(x ? GPIO_SetBits(GPIOF,GPIO_Pin_10) : GPIO_ResetBits(GPIOF,GPIO_Pin_10))	//PF10
#define KEY6_W(x)		(x ? GPIO_SetBits(GPIOF,GPIO_Pin_9)  : GPIO_ResetBits(GPIOF,GPIO_Pin_9))	//PF9
#define KEY7_W(x)		(x ? GPIO_SetBits(GPIOF,GPIO_Pin_8)  : GPIO_ResetBits(GPIOF,GPIO_Pin_8))	//PF8
#define KEY8_W(x)		(x ? GPIO_SetBits(GPIOF,GPIO_Pin_7)  : GPIO_ResetBits(GPIOF,GPIO_Pin_7))	//PF7

//按键值
#define USER1_VAL	100
#define USER2_VAL	101
#define USER3_VAL	102
#define USER4_VAL	103

uint8_t  key_run(void);
void key_Configuration(void);

typedef enum{OFF = 0, ON = !OFF}SWITCH;					//¿ª¹ØÀàÐÍ

#define BEEP(X)		(X ? GPIO_SetBits(GPIOA,GPIO_Pin_8) : GPIO_ResetBits(GPIOA,GPIO_Pin_8))		//PA8


#endif

