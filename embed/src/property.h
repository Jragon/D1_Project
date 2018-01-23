#ifndef PROPERTY_H
#define PROPERTY_H

#include <avr/io.h>
#include "graph.h"
#include "gui.h"

#define MAX_PROPERTIES 3

struct prop;
typedef void (*update_callback_t)(struct prop *, graph_t *);

typedef struct prop {
  uint16_t x, y;
  char *label;
  uint16_t val, fixed_point_divisor;
  graph_dataset_t *dataset;
  update_callback_t update;
} property_t;

typedef struct {
  property_t *p[MAX_PROPERTIES];
  uint8_t count;
  graph_t *graph;
} prop_set_t;

void init_property(uint16_t sample_rate);
void draw_property(property_t *prop);
void draw_plabel(property_t *prop);
void draw_pval(property_t *prop);
int add_property(property_t *prop);
void update_properties();

prop_set_t properties;
volatile int update_property_flag;
int update_graph_flag;

#endif