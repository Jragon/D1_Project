#ifndef GRAPH_H
#define GRAPH_H

#include <avr/io.h>
#include "gui.h"

typedef struct graph_point  {
  uint8_t x, y;
  struct graph_point *next;
} graph_point_t;

typedef struct {
  uint16_t width, height, startx, starty;
  uint8_t draw_point, draw_line;
  uint8_t circle_radius;
  UG_COLOR forecolour, backcolour;
} graph_display_options_t;

typedef struct {
  graph_point_t *head;
  graph_point_t *tail;
  uint8_t size, count;
  graph_display_options_t disp;
} graph_t;


graph_point_t *new_point(uint8_t x, uint8_t y, graph_point_t *next);
graph_point_t *prepend_point(uint8_t x, uint8_t y, graph_point_t *head);
graph_point_t *remove_back(graph_point_t *head);
graph_t create_graph(uint8_t size, uint8_t x, uint8_t y, uint8_t width,
                     uint8_t height);
void clear_graph(graph_t *graph);
void draw_graph(graph_t *graph);
void add_point(uint8_t x, uint8_t y, graph_t *graph);

#endif