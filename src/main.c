#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "lcd.h"

void init_usr_led()
{
    DDRB |= _BV(PB7);
    // initilise off
    PORTB &= ~_BV(PB7);
}

int main (void)
{
    init_uart0();
    init_usr_led();
    init_lcd();

    sei();
    
    char string[15];
    uint8_t data;
    int j;
    int i = 0;
    while(1)
    {
        if(uart_get(&data) == 1){
            if ((char) data != '\n'){
                string[i] = (char) data;
                i++;
            } else {
                for (j = 0; j < i; j++){
                    uart_put_ch(string[j]);
                }
                i = 0;
            }
        } else {
            //display_char('n');
        }
    }
}