#ifndef __RTC_H
#define __RTC_H

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "Gpio.h"
#include "USART.h"

typedef struct   
{ 
  vu8 hour; 
  vu8 min; 
  vu8 sec;   
  //公历日月年周
  vu16 w_year; 
  vu8    w_month;
  vu8    w_date; 
  vu8    week; 
}_calendar_obj; 

extern _calendar_obj calendar;

void RTC_Configuration(void);
uint8_t USART_Scanf(uint32_t value);
uint32_t Time_Regulate(void);
void Time_Adjust(void);
void Time_Display(uint32_t TimeVar);
void RTC_Init(void);
#endif
