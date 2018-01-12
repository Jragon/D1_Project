#include <avr/interrupt.h>
#include <avr/io.h>
#include <math.h>
#include <stdio.h>
#include <util/delay.h>

#include "graph.h"
#include "gui.h"
#include "property.h"
#include "uart.h"

void init_usr_led() { DDRB |= _BV(PB7); }

void voltage_update(property_t *prop) {
  static float lastval = 0;
  static uint8_t count = 0;

  if (trunc(1000. * lastval) != trunc(1000. * prop->val)) {
    draw_pval(prop); 
    lastval = prop->val;
  }

  if (prop->graph != 0) {
    add_point(count, prop->val, prop->graph);
    draw_graph(prop->graph);
    count++;

    update_graph_flag = 1;
  }
}

void setpoint_update(property_t *prop) {
  static uint8_t count = 0;

  if (prop->graph != 0) {
    add_point(count, prop->val, prop->graph);
    draw_graph(prop->graph);
    count++;

    update_graph_flag = 1;
  }
}

int main(void) {
  init_uart0();
  init_usr_led();
  gui_init();
  init_property(3);

  sei();

  property_t setpoint = {.x = 0, .y = 0, .label = "SPT: ", .val = M_E};
  property_t voltage = {.x = 0,
                        .y = gui.font.char_height,
                        .label = "Voltage: ",
                        .val = 0,
                        .update = voltage_update};
  //add_property(&setpoint);
  draw_property(&setpoint);

  add_property(&voltage);
  draw_property(&voltage);

  uint8_t data;
  int set_flag = 0;
  long i = 0;
  float control_output = 0;
  // float last_val = 0;
  // char buffer[15];

  graph_t voltage_graph = create_graph(60, 0, 50, 240, 100);
  voltage.graph = &voltage_graph;
  voltage_graph.disp.draw_line = 1;
  voltage_graph.disp.draw_point = 0;
  // float gradient = (voltage_graph.disp.height / voltage_graph.size);
  // uint16_t y;
  // for (i = 1; i < voltage_graph.size + 1; i++) {
  //   y = (i - 10) * (i - 10);
  //   add_point(0, y, &voltage_graph);
  // }
  // voltage_graph.disp.draw_line = 1;

  // draw_graph(&voltage_graph);

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
            i = 0;
          } else if (data == 's') {
            set_flag = 1;
          }
        }
      } else if (uart_command == GET) {
        // uart_put(&setpoint.val);
        uart_command = NIL;
      }
    }

    // generate value for voltage using setpoint
    control_output = setpoint.val - voltage.val;
    voltage.val += control_output * (1 - (float)pow(M_E, -(i / 100)));
    i++;

    update_properties();

    // snprintf(buffer, 8, "%5.2f; ", control_output);
    // UG_ConsolePutString(buffer);
  }
}