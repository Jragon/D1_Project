#include <avr/interrupt.h>
#include <avr/io.h>
#include "lcd.h"
#include "uart.h"
#include "UGUI/ugui.h"

void init_usr_led() {
  DDRB |= _BV(PB7);
  // initilise off
  PORTB &= ~_BV(PB7);
}

void lcd_pset(UG_S16 x, UG_S16 y, UG_COLOR c) { 
  draw_pixel(x, y, c); 
}

UG_GUI gui;

int main(void) {
  init_uart0();
  init_usr_led();

  init_lcd();
  UG_Init(&gui, lcd_pset, 240, 320);
  UG_FontSelect(&FONT_6X8);

  sei();

  unsigned char string[15];
  uint8_t data;
  int j;
  int i = 0;

  UG_ConsoleSetArea(0, 120, 240, 200);
  UG_ConsoleSetForecolor(C_GREEN_YELLOW);
  UG_ConsoleSetBackcolor(C_BLACK);
  UG_ConsolePutString("Hey!");

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