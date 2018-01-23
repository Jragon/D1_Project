#ifndef GRAPH_H
#define GRAPH_H

#include <avr/io.h>
#include "gui.h"

#define MAX_DATA_SET_PER_GRAPH 3
#define GRAPH_SCALE_DIVISOR 100
#define GRAPH_X_OFFSET_DIVISOR 100

typedef struct graph_point {
  uint16_t x, y;
  struct graph_point *next;
} graph_point_t;

typedef struct {
  uint16_t width, height, startx, starty, legend_height;
  uint8_t draw_point, draw_line, draw_legend;
  uint8_t circle_radius, margin;
  UG_COLOR forecolour, backcolour, outline_colour;
} graph_display_options_t;

typedef struct {
  char *name;
  graph_point_t *head;
  uint8_t count, redraw;
  uint16_t finalval, maxy;
  UG_COLOR colour;
} graph_dataset_t;

typedef struct {
  char *title;
  graph_dataset_t *dataset[MAX_DATA_SET_PER_GRAPH];
  uint8_t size, maxvalue, dataset_count;
  graph_display_options_t disp;
} graph_t;

graph_point_t *new_point(uint16_t x, uint16_t y, graph_point_t *next);
graph_point_t *prepend_point(uint16_t x, uint16_t y, graph_point_t *head);
graph_point_t *remove_back(graph_dataset_t *dataset);
graph_t create_graph(uint8_t size, uint16_t x, uint16_t y, uint16_t width,
                     uint16_t height);
void clear_graph(graph_t *graph);
void draw_graph(graph_t *graph);
void add_point(uint16_t x, uint16_t y, graph_t *graph,
               graph_dataset_t *dataset);

graph_dataset_t create_dataset(char *name, UG_COLOR colour);
void add_dataset(graph_t *graph, graph_dataset_t *dataset);
void draw_dataset_points(graph_t *graph, graph_dataset_t *dataset,
                         UG_COLOR colour);

void draw_graph_legend(graph_t *graph);

#endif