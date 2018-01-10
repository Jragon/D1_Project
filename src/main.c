#include <avr/io.h>
#include <avr/interrupt.h>
#include "ring_buff.h"
#include "lcd.h"

#define BAUD 9600
#define UART_PRESCALE (F_CPU/(BAUD*16L) - 1)

static rbIndex_t _rxBuffIndex;
static uint8_t _rxBuffMem[16];

// receive bit isr
ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;
    uint8_t data2;
    if(ring_buffer_put(_rxBuffIndex, &data) == 1)
        PINB |= _BV(PB7);
}

void init_uart0()
{
    UCSR0B = _BV(RXEN0) | _BV(TXEN0); // receive / transmit enable
    UCSR0B |= _BV(RXCIE0); // receive complete interrupt enable
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // 8-bit char size

    // set usart prescale register
    UBRR0H = UART_PRESCALE >> 8; // shift down 8 bits 
    UBRR0L = UART_PRESCALE;

    // init ring buffs
    ring_buffer_init(&_rxBuffIndex, 16, _rxBuffMem);
}

int uart_get(uint8_t *data)
{
    uint8_t val;

    if (ring_buffer_get(_rxBuffIndex, &val) == 1) {
        *data = val;
        return 1;
    } else {
        return 0;
    }
}

void uart_put_ch(char ch){
    if (ch == '\n')
        uart_put_ch('\r');

    while (!(UCSR0A & _BV(UDRE0)));
    UDR0 = ch;
}

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