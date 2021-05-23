#ifndef __USART_H
#define __USART_H

#include <stm32f10x.h>
#include "stdio.h"

#define DATA_BUF_SIZE							64

extern uint16_t data_length;
extern uint8_t rx_buffer[];
extern uint8_t tx_buffer[];

void USART1_Init(void);
void USART1_SendByte(uint16_t dat);
void USART1_SendString(uint8_t *buf,uint16_t len);

void USART2_Init(void);
void USART2_SendByte(uint16_t dat);
void USART2_SendString(uint8_t *ch);
void SendMessage(void);
#endif
