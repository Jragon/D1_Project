#ifndef PROPERTY_H
#define PROPERTY_H

#include <avr/io.h>
#include "gui.h"
#include "graph.h"

#define MAX_PROPERTIES 2

struct prop;
typedef void (* update_callback_t)(struct prop *);

typedef struct prop {
  uint16_t x, y;
  char *label;
  float val;
  graph_t *graph;
  update_callback_t update;
} property_t;

void init_property(uint16_t sample_rate);
void draw_property(property_t *prop);
void draw_plabel(property_t *prop);
void draw_pval(property_t *prop);
int add_property(property_t *prop);
void update_properties();

volatile property_t *properties[MAX_PROPERTIES];
volatile int update_property_flag;
int update_graph_flag;

#endif