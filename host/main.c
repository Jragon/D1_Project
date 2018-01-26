#ifdef _WIN32
#include <Windows.h>
#define uartsleep Sleep(100)
#else
#include <unistd.h>
#define uartsleep usleep(100000)
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rs232.h"
#include "timer.h"

#define BUFF_SIZE 4096
#define VOLTAGE_SCALAR 100

struct {
  unsigned char buff[BUFF_SIZE];
  int comport, baudrate, n, timeout;
  char mode[4];
} uart = {.comport = 2,
          .baudrate = 9600,
          .mode = {'8', 'N', '1', 0},
          .n = 0,
          .timeout = 10};

typedef enum { SET, GET, CONN, TEST } command_t;
typedef enum { UNKNOWN_VAR, SPT, VOLT, PWM } variable_command_t;
unsigned char uart_command_char[4] = {'s', 'g', 'c', 't'};
unsigned char uart_variable_char[4] = {0, 't', 'v', 'p'};

void uart_timer_callback(void);
int uart_test_connection();
int uart_read_byte(uint8_t *val, int n);
int uart_get_val(uint8_t *val);
int uart_get_val_16(uint16_t *val, uint8_t *var, variable_command_t var_name);
int uart_set_val(uint16_t val);
char *getline(void);
char *getline_uart(void);
    int parse_commands();
char *decodeVariable(uint8_t *var);


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

  while (1) {
    parse_commands();
  }

  if (uart_set_val(12)) {
    printf("Error setting value\n");
  } else {
    printf("Set value\n");
  }

  if (uart_get_val(&val)) {
    printf("did not get val: %d\n", val);
  } else {
    printf("get val: %d\n", val);
  }

  return 0;
}

int uart_test_connection() {
  int i = 0, conn = -1;

  if (RS232_SendByte(uart.comport, uart_command_char[CONN])) return conn;

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

int uart_set_val(uint16_t val) {
  int ret = 0;
  uint8_t returned_val;

  RS232_SendByte(uart.comport, uart_command_char[SET]);
  RS232_SendByte(uart.comport, (uint8_t)(val >> 8));    // upper byte first
  RS232_SendByte(uart.comport, (uint8_t)(val & 0xFF));  // lower byte second

  uartsleep;

  if (uart_get_val(&returned_val)) {
    ret = -1;
  } else {
    if (returned_val != val) ret = -1;
  }

  return ret;
}

int uart_timeout(int n) {
  int ret = 0, i = 0;

  while (uart.n < n) {
    if (i > uart.timeout) {
      ret = -1;
      break;
    }

    uartsleep;
    i++;
  }

  return ret;
}

int uart_read_byte(uint8_t *val, int n) {
  int ret = 0, i = 0;

  uart_timeout(n);

  if (ret == 0) *val = uart.buff[0];

  return ret;
}

int uart_get_val(uint8_t *val) {
  int n = uart.n + 1, ret = 0;
  RS232_SendByte(uart.comport, uart_command_char[GET]);

  ret = uart_read_byte(val, n);

  return ret;
}

int uart_get_val_16(uint16_t *val, uint8_t *var, variable_command_t var_name) {
  uint8_t ret,
      n = uart.n + 5;  // wait for 5 bits

  RS232_SendByte(uart.comport, uart_command_char[GET]);
  uartsleep;
  RS232_SendByte(uart.comport, uart_variable_char[var_name]);
  uartsleep;

  uart_timeout(n);

  if (uart.buff[0] == uart_command_char[GET] && uart.buff[4] == '\0') {
    *var = uart.buff[1];
    *val = uart.buff[2] << 8 | uart.buff[3];
  } else {
    ret = -1;
  }

  return ret;
}

void uart_timer_callback(void) {
  uart.n += RS232_PollComport(uart.comport, uart.buff, BUFF_SIZE);
}

int parse_commands() {
  char *line, *token, *delim = " \n";
  float value = 0;
  uint16_t get_val;
  uint8_t get_var;
  variable_command_t var;

  printf("> ");
  fflush(stdout);

  line = getline();
  if (line == NULL) return -1;

  token = strtok(line, delim);
  if (!strcmp(token, "set")) {
    token = strtok(NULL, delim);

    if (sscanf(token, "%f", &value))
      uart_set_val((uint16_t)(value * VOLTAGE_SCALAR));
  } else if (!strcmp(token, "get")) {
    token = strtok(NULL, delim);

    if (!strcmp(token, "setpoint"))
      var = SPT;
    else if (!strcmp(token, "voltage"))
      var = VOLT;
    else if (!strcmp(token, "pwm"))
      var = PWM;
    else
      var = UNKNOWN_VAR;

    uart_get_val_16(&get_val, &get_var, var);
    if (var == PWM)
      printf("%s: %u\n", decodeVariable(&get_var), get_val);
    else
      printf("%s: %5.2fV\n", decodeVariable(&get_var),
             ((float) get_val/VOLTAGE_SCALAR));
  } else if (!strcmp(token, "test")) {
    int n = uart.n;
    RS232_SendByte(uart.comport, uart_command_char[TEST]);
    uartsleep;
    uartsleep;
    line = getline_uart();
    while(strcmp(line, "END")){
      uart_timeout(n += 10);
      line = getline_uart();
      printf("%s", line);
    }
  } else {
    printf("Unrecognised command\n");
  }

  return 0;
}

char *decodeVariable(uint8_t *var) {
  if ((char)*var == uart_variable_char[SPT])
    return "Setpoint";
  else if ((char)*var == uart_variable_char[VOLT])
    return "Voltage";
  else if ((char)*var == uart_variable_char[PWM])
    return "PWM OCR2A";
  else
    return "Unknown Var";
}

char *getline(void) {
  char *line = malloc(100), *linep = line;
  size_t lenmax = 100, len = lenmax;
  int c;

  if (line == NULL) return NULL;

  while (1) {
    c = fgetc(stdin);
    if (c == EOF) break;

    if (--len == 0) {
      len = lenmax;
      char *linen = realloc(linep, lenmax *= 2);

      if (linen == NULL) {
        free(linep);
        return NULL;
      }
      line = linen + (line - linep);
      linep = linen;
    }

    if ((*line++ = c) == '\n') break;
  }

  *line = '\0';
  return linep;
}

char *getline_uart(void) {
  char *line = malloc(100), *linep = line;
  size_t lenmax = 100, len = lenmax;
  int c, i;

  if (line == NULL) return NULL;

  while (1) {
    c = uart.buff[i];
    if (c == EOF) break;

    if (--len == 0) {
      len = lenmax;
      char *linen = realloc(linep, lenmax *= 2);

      if (linen == NULL) {
        free(linep);
        return NULL;
      }
      line = linen + (line - linep);
      linep = linen;
    }

    if ((*line++ = c) == '\n') break;
    i++;

  }

  *line = '\0';
  return linep;
}