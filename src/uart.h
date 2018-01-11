#ifndef UART_H
#define UART_H

#include "ring_buff.h"

#define UART_BAUD 9600
#define UART_PRESCALE (F_CPU / (UART_BAUD * 16L) - 1)
#define UART_BUFFER_SIZE 16

void init_uart0();

int uart_get(uint8_t *data);
int uart_put(uint8_t *data);

void uart_put_ch(char ch);

#endif