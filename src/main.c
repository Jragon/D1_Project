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

  uint8_t data;
  int set_flag = 0;

  while (1) {
    if (uart_command != NIL) {
      if (uart_command == SET) {
        if (uart_get(&data)) {
          if (set_flag) {
            UG_ConsolePutString("\nSET: ");
            console_put_number(data);
            setpoint.val = data;
            draw_pval(&setpoint);
            uart_command = NIL;
            set_flag = 0;
          } else if (data == 's') {
            set_flag = 1;
          }
        }
      } else if (uart_command == GET) {
        uart_put(&setpoint.val);
        uart_command = NIL;
      }
    }
  }
}