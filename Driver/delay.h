#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void delay_us(uint16_t time) ;
void delay_ms(uint16_t time) ;

void Delay_Init(void);
void Delay_ms(uint32_t nms);
void Delay_us(uint32_t nus);

#endif
