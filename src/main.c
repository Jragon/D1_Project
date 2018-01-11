#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#include "uart.h"
#include "gui.h"


void init_usr_led() {
  DDRB |= _BV(PB7);
  // initilise off
  PORTB &= ~_BV(PB7);
}

int main(void) {
  init_uart0();
  init_usr_led();
  gui_init();
  
  sei();

  Property setpoint = {.x = 0, .y = 0, .label = "SPT: ", .val = 0};

  drawProperty(&setpoint);

  char buffer[100];

  for (uint8_t i = 0; i < 25; i++) {
    snprintf(buffer, 100, "hello %d\n", i);
    UG_ConsolePutString(buffer);
  }

  unsigned char string[15];
  uint8_t data;
  int j;
  int i = 0;

  while (1) {
    if (uart_get(&data) == 1) {
      if ((char)data != '\n') {
        string[i] = (char)data;
        i++;
      } else {
        for (j = 0; j < i; j++) {
          uart_put((uint8_t *)&string[j]);
        }
        i = 0;
      }
    } else {
      // display_char('n');
    }
  }
}