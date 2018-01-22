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

void init_usr_led() { DDRB |= _BV(PB7); }

void voltage_update(property_t *prop, graph_t *graph) {
  static uint16_t lastval = 0;

  prop->val = adc_read();

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
    add_point(count, sin(count * 0.25) * 15 + 30, graph, prop->dataset);
    update_graph_flag = 1;
    count++;
  }
}

int main(void) {
  init_uart0();
  init_usr_led();
  gui_init();
  init_property(20);
  init_rotary();
  init_pwm();
  init_adc();

  set_pwm_duty(255);

  sei();

  ADC_START;

  graph_t maingraph = create_graph(150, 0, 50, 240, 100);
  maingraph.disp.draw_line = 1;
  maingraph.disp.draw_point = 0;
  maingraph.title = "Tiotal1";

  properties.graph = &maingraph;

  console_put_number(maingraph.dataset_count);

  graph_dataset_t voltage_dataset = create_dataset("Voltage", C_YELLOW);
  voltage_dataset.maxy = 1100;
  add_dataset(&maingraph, &voltage_dataset);

  graph_dataset_t setpoint_dataset = create_dataset("Setpoint", C_BLUE);
  add_dataset(&maingraph, &setpoint_dataset);

  draw_graph_legend(&maingraph);

  console_put_number(maingraph.dataset_count);
  UG_ConsolePutString(maingraph.title);
  UG_ConsolePutString("\n");

  property_t setpoint = {.x = 0,
                         .y = 0,
                         .label = "SPT: ",
                         .val = M_E,
                         .update = setpoint_update,
                         .dataset = &setpoint_dataset};
  property_t voltage = {.x = 0,
                        .y = gui.font.char_height,
                        .label = "Voltage: ",
                        .val = 0,
                        .update = voltage_update,
                        .dataset = &voltage_dataset};
  add_property(&setpoint);
  draw_property(&setpoint);

  add_property(&voltage);
  draw_property(&voltage);

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
        uint8_t sp = (uint8_t)trunc(setpoint.val);
        uart_put(&sp);
        UG_ConsolePutString("\nSending: ");
        console_put_number(sp);
        uart_command = NIL;
      } else if (uart_command == CONN) {
        if (uart_put(&uart_command_char[uart_command]))
          UG_ConsolePutString("\n** Connection Successful ** \n");
        else
          UG_ConsolePutString("\n** Connection Error ** \n");

        uart_command = NIL;
      }
    }

    update_properties();
  }
}