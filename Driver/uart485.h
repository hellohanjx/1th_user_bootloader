#ifndef UART_485_H_
#define UART_485_H_
#include "stdint.h"

enum{APP = 1, UPDATA, WAIT};
	
void uart_485_config(void);
uint8_t handshake_with_driver(uint8_t cmd, uint8_t *dat, uint16_t datLen);


#endif

