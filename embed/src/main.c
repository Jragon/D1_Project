#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdio.h>
#include <util/delay.h>

#include "adc.h"
#include "graph.h"
#include "gui.h"
#include "property.h"
#include "pwm.h"
#include "rotary.h"
#include "uart.h"

#define PROP_UPDATE_FREQ 100
#define PROP_UPDATE_TIME 1 / PROP_UPDATE_FREQ
#define SCALE_DIVISOR 100
#define VOLTAGE_SCALE 100
#define ADCMAX 1023
#define VREF 3.3

// voltage divider resistors (k ohms)
#define VD_R1 21.9850
#define VD_R2 4.6735
// VOLTAGE_DIV = (R1 + R2) / R2
#define VOLTAGE_DIV 5.7041

char *var_ch_string(uint8_t *var);
variable_command_t var_ch_enum(uint8_t *var);

void init_usr_led() { DDRB |= _BV(PB7); }

int test_pwm_flag = 0;

int calculate_output(uint16_t voltage, uint16_t setpoint, uint16_t tolerance) {
  static int last_error;
  static long int integral = 0;
  int Kp = 5, Ki = 2, Kd = 3;

  int error = (int)(setpoint - voltage);

  // d = (error-last)/dt === (error-last) * freq
  // time should be fiarly constant
  int derivative = (error - last_error);
  integral = error < tolerance ? 0 : integral + error;

  last_error = error;

  return (Kp * error + (Ki * integral) + Kd * derivative) / SCALE_DIVISOR;
}

float adc_to_volts(int adc) {
  return ((float)adc * VREF / ADCMAX) * (VD_R1 + VD_R2) / VD_R2;
}

uint16_t volts_to_adc(float voltage) {
  return (uint16_t)(voltage / (VD_R1 + VD_R2) * VD_R2 * ADCMAX / VREF);
}

void send_status_uart() {
  int i, n;
  char buff[100];

  for (i = 0; i < MAX_PROPERTIES; i++) {
    n = snprintf(buff, 100, "%u, ", properties.p[i]->val);
    if (n > 0)
      uart_put_array((uint8_t *)buff, n);
    else
      UG_ConsolePutString("Error Sending UART Status\n");
  }

  uart_put_array((uint8_t *)"\n", 1);
}

void voltage_update(property_t *prop, graph_t *graph) {
  static uint16_t lastval = 0;

  prop->val = adc_to_volts(adc_read()) * prop->fixed_point_divisor;

  // if (trunc(1000. * lastval) != trunc(1000. * prop->val)) {
  if (lastval != prop->val) {
    lastval = prop->val;
    draw_pval(prop);
  }

  if (prop->dataset != 0) {
    add_point(prop->dataset->count, prop->val, graph, prop->dataset);
    update_graph_flag = 1;
  }
}

void setpoint_update(property_t *prop, graph_t *graph) {
  static uint8_t count = 0;
  if (prop->dataset != 0) {
    // draw sine graph
    // add_point(count, sin(count * 0.25) * 15 + 30, graph, prop->dataset);
    add_point(count, prop->val, graph, prop->dataset);
    update_graph_flag = 1;
    count++;
  }
}

void pwm_update(property_t *prop, graph_t *graph) {
  static uint16_t lastval = 0;
  static uint8_t divider = 0;

  if (test_pwm_flag) {
    send_status_uart();

    if (prop->val < PWM_DUTY_MAX) {
      // freq div
      if (divider < 2) {
        divider++;
      } else {
        prop->val++;
        divider = 0;
      }
    } else {
      uart_put_array((uint8_t *)"END\n", 5);
      prop->val = 0;
      uart_command = NIL;
      test_pwm_flag = 0;
    }
  } else {
    uint16_t output = prop->val + calculate_output(properties.p[1]->val,
                                                   properties.p[0]->val, 50);

    prop->val = output < PWM_DUTY_MAX ? output : PWM_DUTY_MAX;
    // prop->val = (properties.p[1]->val < properties.p[0]->val)
    //                 ? prop->val < PWM_DUTY_MAX ? prop->val + 1 : prop->val
    //                 : prop->val != 0 ? prop->val - 1 : prop->val;
  }

  set_pwm_duty(prop->val);

  if (lastval != prop->val) {
    lastval = prop->val;
    draw_pval(prop);
  }

  if (prop->dataset != 0) {
    add_point(prop->dataset->count, prop->val, graph, prop->dataset);
    update_graph_flag = 1;
  }
}

int main(void) {
  init_uart0();
  init_usr_led();
  gui_init();
  init_property(PROP_UPDATE_FREQ);
  // init_rotary();
  init_pwm();
  init_adc();
  sei();

  ADC_START;

  graph_t maingraph = create_graph(100, 0, 50, 240, 100);
  maingraph.disp.draw_line = 1;
  maingraph.disp.draw_point = 0;
  maingraph.title = "Tiotal1";

  properties.graph = &maingraph;

  graph_dataset_t voltage_dataset = create_dataset("Voltage", C_YELLOW);
  voltage_dataset.maxy = 18 * VOLTAGE_SCALE;
  add_dataset(&maingraph, &voltage_dataset);

  graph_dataset_t setpoint_dataset = create_dataset("Setpoint", C_BLUE);
  setpoint_dataset.maxy = 18 * VOLTAGE_SCALE;
  add_dataset(&maingraph, &setpoint_dataset);

  graph_dataset_t pwm_dataset = create_dataset("PWM Duty", C_ORANGE_RED);
  pwm_dataset.maxy = PWM_DUTY_MAX + 50;
  add_dataset(&maingraph, &pwm_dataset);

  draw_graph_legend(&maingraph);

  property_t setpoint = {.x = 0,
                         .y = 0,
                         .label = "SPT: ",
                         .val = 550,
                         .update = setpoint_update,
                         .dataset = &setpoint_dataset,
                         .fixed_point_divisor = VOLTAGE_SCALE};

  add_property(&setpoint);
  draw_property(&setpoint);

  property_t voltage = {.x = 0,
                        .y = gui.font.char_height + 1,
                        .label = "Voltage: ",
                        .val = 0,
                        .update = voltage_update,
                        .dataset = &voltage_dataset,
                        .fixed_point_divisor = VOLTAGE_SCALE};

  add_property(&voltage);
  draw_property(&voltage);

  property_t pwm_output = {.x = 0,
                           .y = (gui.font.char_height + 1) * 2,
                           .label = "PWM Duty: ",
                           .val = 0,
                           .update = pwm_update,
                           .dataset = &pwm_dataset,
                           .fixed_point_divisor = 0};
  add_property(&pwm_output);
  draw_property(&pwm_output);

  UG_ConsolePutString("*** Boost initialised ***\n");

  uint8_t data8;
  uint16_t set_voltage, get_return;
  uint8_t sp[5];
  int set_flag = 0, var_name = 0;

  while (1) {
    if (uart_command != NIL) {
      if (uart_command == SET) {
        if (uart_get(&data8)) {
          if (set_flag) {
            set_voltage = data8 << 8;
            while (!uart_get(&data8))
              ;
            set_voltage |= data8;

            UG_ConsolePutString("\nSET: ");
            console_put_float((float)set_voltage / VOLTAGE_SCALE);
            setpoint.val = set_voltage;
            draw_pval(&setpoint);
            uart_command = NIL;
            set_flag = 0;
          } else if (data8 == 's') {
            set_flag = 1;
          }
        }
      } else if (uart_command == GET) {
        if (!var_name) {
          if (uart_get(&data8)) {
            var_name = var_ch_enum(&data8);
          }
        } else {
          sp[0] = uart_command_char[GET];        // get command var
          sp[1] = uart_variable_char[var_name];  // getting sp

          if (var_name == SPT)
            get_return = setpoint.val;
          else if (var_name == VOLT)
            get_return = voltage.val;
          else if (var_name == PWM)
            get_return = pwm_output.val;
          else
            get_return = 0;

          sp[2] = get_return >> 8;
          sp[3] = get_return & 0xFF;
          sp[4] = '\0';

          uart_put_array(sp, 5);
          UG_ConsolePutString("\nSending: ");
          UG_ConsolePutString(var_ch_string(&uart_variable_char[var_name]));
          UG_ConsolePutString(" - ");

          if (var_name == PWM)
            console_put_number(get_return);
          else
            console_put_float(((float)get_return / VOLTAGE_SCALE));

          uart_command = NIL;
          var_name = 0;
        }
      } else if (uart_command == CONN) {
        if (uart_put(&uart_command_char[uart_command]))
          UG_ConsolePutString("\n** Connection Successful ** \n");
        else
          UG_ConsolePutString("\n** Connection Error ** \n");

        uart_command = NIL;
      } else if (uart_command == TEST) {
        if (!test_pwm_flag) {
          test_pwm_flag = 1;
          UG_ConsolePutString("\n** Starting PWM Test **\n");
          pwm_output.val = 0;
          // leave uart_command = NIL;
        }
      }
    }

    update_properties();
  }
}

char *var_ch_string(uint8_t *var) {
  if ((char)*var == uart_variable_char[SPT])
    return "Setpoint";
  else if ((char)*var == uart_variable_char[VOLT])
    return "Voltage";
  else if ((char)*var == uart_variable_char[PWM])
    return "PWM OCR2A";
  else
    return "Unknown Var";
}

variable_command_t var_ch_enum(uint8_t *var) {
  if ((char)*var == uart_variable_char[SPT])
    return SPT;
  else if ((char)*var == uart_variable_char[VOLT])
    return VOLT;
  else if ((char)*var == uart_variable_char[PWM])
    return PWM;
  else
    return UNKNOWN_VAR;
}