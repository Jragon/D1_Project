#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#include "gui.h"
#include "uart.h"

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

  draw_property(&setpoint);

  int i = 0;
  uint8_t data;
  char buffer[100];

  while (1) {
    if (uart_command != NIL) {
      if (uart_command == SET) {
        UG_ConsolePutString("\nSET\n");
        if (uart_get(&data)) {
          snprintf(buffer, 100, "%d; ", data);
          UG_ConsolePutString(buffer);

          if (data != 's') {
            setpoint.val = data;
            draw_pval(&setpoint);
            uart_command = NIL;
          }
        } else {
          uart_command = NIL;
        }
      } else if (uart_command == GET) {
        uart_put(&setpoint.val);
        uart_command = NIL;
      }
    }
  }
}