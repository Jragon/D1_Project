#ifndef UART_H
#define UART_H

#include "ring_buff.h"

#define BAUD 9600
#define UART_PRESCALE (F_CPU / (BAUD * 16L) - 1)

void init_uart0();
int uart_get(uint8_t *data);
void uart_put_ch(char ch);

#endif