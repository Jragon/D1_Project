#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#define uartsleep Sleep(100)
#else
#include <unistd.h>
#define uartsleep usleep(100000)
#endif

#include "rs232.h"
#include "timer.h"

#define BUFF_SIZE 4096

struct {
  unsigned char buff[BUFF_SIZE];
  int comport, baudrate, n, timeout;
  char mode[4];
} uart = {
    .comport = 2, .baudrate = 9600, .mode = {'8', 'N', '1', 0}, .n = 0, .timeout = 10
};

typedef enum { SET, GET, CONN } command_t;
unsigned char uart_command_char[3] = {'s', 'g', 'c'};

void uart_timer_callback(void);
int uart_test_connection();
int uart_get_val(uint8_t *val);
int uart_set_val(uint8_t val);

int main() {
  uint8_t val = 0;

  if (RS232_OpenComport(uart.comport, uart.baudrate, uart.mode)) {
    printf("Cannot open comport \n");
    return 0;
  }

  if (start_timer(200, uart_timer_callback)) {
    printf("Timer initialization error\n");
    return 0;
  }

  if (uart_test_connection()) {
    printf("Connection Error\n");
    return 0;
  } else {
    printf("Connection Successful!\n");
  }

  if(uart_set_val(12)){
    printf("Error setting value\n");
  } else {
    printf("Set value\n");
  }

  if(uart_get_val(&val)){
    printf("did not get val: %d\n", val);
  } else {
    printf("get val: %d\n", val);
  }

  return 0;
}

int uart_test_connection() {
  int i = 0, conn = -1;

  if(RS232_SendByte(uart.comport, uart_command_char[CONN]))
    return conn;

  while (uart.n < 1) {
    if (i > uart.timeout) {
      conn = -1;
      break;
    }

    uartsleep;
    i++;
  }

  for (int j = 0; j < uart.n; ++j) {
    if (uart.buff[j] == uart_command_char[CONN]) {
      conn = 0;
      break;
    }
  }

  return conn;
}

int uart_set_val(uint8_t val) {
  int ret = 0;
  uint8_t returned_val;

  RS232_SendByte(uart.comport, uart_command_char[SET]);
  RS232_SendByte(uart.comport, val);

  uartsleep;

  if (uart_get_val(&returned_val)) {
    ret = -1;
  } else {
    if (returned_val != val)
      ret = -1;
  }

  return ret;
}

int uart_get_val(uint8_t *val) {
  int n = uart.n + 1, ret = 0, i = 0;
  RS232_SendByte(uart.comport, uart_command_char[GET]);

  while (uart.n < n) {
    if (i > uart.timeout) {
      ret = -1;
      break;
    }

    uartsleep;
    i++;
  }

  if (ret == 0)
    *val = uart.buff[0];

  return ret;
}

void uart_timer_callback(void) {
  uart.n += RS232_PollComport(uart.comport, uart.buff, BUFF_SIZE);
}