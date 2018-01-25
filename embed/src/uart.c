#include <avr/interrupt.h>
#include <avr/io.h>

#include "uart.h"

static rbIndex_t _rxBuffIndex, _txBuffIndex;
static uint8_t _rxBuffMem[UART_BUFFER_SIZE], _txBuffMem[UART_BUFFER_SIZE];

uint8_t uart_command_char[4] = {0, 's', 'g', 'c'};
uint8_t uart_variable_char[4] = {0, 't', 'v', 'p'};

// receive bit isr
ISR(USART0_RX_vect) {
  uint8_t data = UDR0;
  if (ring_buffer_put(_rxBuffIndex, &data) == 1) {
    if (uart_command == NIL) {
      if (data == 's') {
        uart_command = SET;
      } else if (data == 'g') {
        uart_command = GET;
      } else if (data == 'c') {
        uart_command = CONN;
      }
    }
  }
}

// UDR0 empty isr, transmit
ISR(USART0_UDRE_vect) {
  uint8_t data;
  // if there is data
  if (ring_buffer_get(_txBuffIndex, &data) == 1) {
    UDR0 = data;
  } else {
    // disable the interrupt because all the data has been transmitted
    UCSR0B &= ~_BV(UDRIE0);
  }
}

void init_uart0() {
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);    // receive / transmit enable
  UCSR0B |= _BV(RXCIE0);               // receive complete interrupt enable
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);  // 8-bit char size

  // set usart prescale register
  UBRR0H = UART_PRESCALE >> 8;  // shift down 8 bits
  UBRR0L = UART_PRESCALE;

  // init ring buffs
  ring_buffer_init(&_rxBuffIndex, UART_BUFFER_SIZE, _rxBuffMem);
  ring_buffer_init(&_txBuffIndex, UART_BUFFER_SIZE, _txBuffMem);
}

int uart_get(uint8_t *data) {
  uint8_t val;

  if (ring_buffer_get(_rxBuffIndex, &val) == 1) {
    *data = val;
    return 1;
  } else {
    return 0;
  }
}

int uart_get_16(uint16_t *data) {
  uint8_t val1, val2;

  if (ring_buffer_get(_rxBuffIndex, &val1) == 1 &&
      ring_buffer_get(_rxBuffIndex, &val2) == 1) {
    *data = val1 << 8 | val2;
    return 1;
  } else {
    return 0;
  }
}

int uart_put(uint8_t *data) {
  if (ring_buffer_put(_txBuffIndex, data) == 1) {
    // enable transfer complete interrupt
    // the interrupt fires if UDR0 is empty, which means as soon
    // as the interrupt is enabled they the interrupt will start
    UCSR0B |= _BV(UDRIE0);
    return 1;
  } else {
    // ring buffer full or something
    return 0;
  }
}

int uart_put_array(uint8_t *data, int count) {
  int i, ret = 1;
  for (i = 0; i < count; i++) {
    if (uart_put(data) == 0) ret = 0;

    data++;
  }

  return ret;
}

void uart_put_ch(char ch) {
  if (ch == '\n') uart_put_ch('\r');

  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = ch;
}
