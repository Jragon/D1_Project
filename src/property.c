#include <avr/interrupt.h>
#include <stdio.h>

#include "property.h"

static volatile uint8_t property_count = 0;

ISR(TIMER1_COMPA_vect) {
  update_property_flag = 1;
}

void init_property(uint16_t sample_rate) {
  TCCR1B = _BV(WGM12); // CTC Mode
  TCCR1B |= _BV(CS12); // Prescaler: F_CPU / 256

  // F_CPU/prescalar = 12000000/64 = 187500 ticks per second
  // capable of doing 1Hz
  OCR1A = (uint16_t)(F_CPU / (2 * 64.0 * sample_rate) - 1);

  // enable interrupt flag
  TIMSK1 |= _BV(OCIE0A);

  update_property_flag = 0;
  update_graph_flag = 0;
}

int add_property(property_t *prop) {
  if (property_count < MAX_PROPERTIES) {
    properties[property_count] = prop;
    property_count++;
  } else {
    // no more space in the properties array
    return 0;
  }

  return 1;
}

void draw_property(property_t *prop) {
  draw_plabel(prop);
  draw_pval(prop);
}

void draw_plabel(property_t *prop) {
  UG_PutString(prop->x, prop->y, prop->label);
}

void draw_pval(property_t *prop) {
  char valstr[15];
  snprintf(valstr, 15, "%10.8f", prop->val);

  UG_PutString(prop->x + strwidth(prop->label), prop->y, valstr);
}

void update_properties() {
  if (update_property_flag == 1) {
    // disable interrupt
    TIMSK1 &= ~_BV(OCIE0A);

    for(int i = 0; i < property_count; i++) {
      properties[i]->update((property_t *) properties[i]);
    }

    if (update_graph_flag == 1)
      update_graph();

    update_property_flag = 0;
    // enable interrupt
    TIMSK1 |= _BV(OCIE0A);
  }
}